#include "MentionManager.hpp"

#include <arc/prelude.hpp>
#include <fmt/format.h>
#include <utils.hpp>
#include <filtering.hpp>

#include <Geode/Geode.hpp>
#include <Geode/ui/Notification.hpp>
#include <Geode/utils/base64.hpp>
#include <Geode/utils/web.hpp>
#include <Geode/utils/random.hpp>
#include <Geode/utils/string.hpp>

#include <ranges>
#include <string>
#include <vector>

#include <xblazegmd.geode-api/include/XblazeAPI.hpp>

using namespace geode::prelude;

MentionManager::MentionManager() {
    m_previousMentions = std::ranges::to<std::deque<CommentObject>>(
        Mod::get()->getSavedValue<std::vector<CommentObject>>("mentions")
    );
};

MentionManager* MentionManager::get() {
    static MentionManager* instance = new MentionManager();
    return instance;
}

void MentionManager::start() {
    m_watcher.spawn(
        "MentionManager::mentionTracker",
        commentWatcher(),
        [] {}
    );
}

void MentionManager::save() {
    log::debug("Saving mentions...");
    Mod::get()->setSavedValue("mentions", std::ranges::to<std::vector<CommentObject>>(m_previousMentions));
    log::debug("Successfully saved mentions");
}

arc::Future<> MentionManager::fetchDailyID() {
    auto levelID = co_await getSpecialID(LevelType::Daily);
    if (levelID.isErr()) {
        notifyError(fmt::format("CommentMentions: Could not get daily level's ID: {}", levelID.unwrapErr()));
        co_return;
    }
    auto lock = co_await m_dailyID.lock();
    *lock = std::move(levelID).unwrap();
}

arc::Future<> MentionManager::fetchWeeklyID() {
    auto levelID = co_await getSpecialID(LevelType::Weekly);
    if (levelID.isErr()) {
        notifyError(fmt::format("CommentMentions: Could not get weekly level's ID: {}", levelID.unwrapErr()));
        co_return;
    }
    auto lock = co_await m_weeklyID.lock();
    *lock = std::move(levelID).unwrap();
}

arc::Future<> MentionManager::fetchEventID() {
    auto levelID = co_await getSpecialID(LevelType::Event);
    if (levelID.isErr()) {
        notifyError(fmt::format("CommentMentions: Could not get event level's ID: {}", levelID.unwrapErr()));
        co_return;
    }
    auto lock = co_await m_eventID.lock();
    *lock = std::move(levelID).unwrap();
}

arc::Future<> MentionManager::loadCustomIDs() {
    auto lock = co_await m_customIDs.lock();
    lock->clear();

    if (!Mod::get()->getSettingValue<bool>("use-custom-ids")) co_return;

    auto customIDs = Mod::get()->getSettingValue<std::string>("custom-ids");
    auto ids = string::split(customIDs, ",");

    for (const auto& id : ids) {
        auto idNum = utils::numFromString<int>(string::trim(id));
        if (idNum.isErr()) {
            log::error("Error converting ID {} to number: {}", id, idNum.unwrapErr());
            continue;
        }
        lock->push_back(std::move(idNum).unwrap());
    }
}

arc::Future<std::optional<int>> MentionManager::getDailyID() {
    auto lock = co_await m_dailyID.lock();
    co_return *lock;
}

arc::Future<std::optional<int>> MentionManager::getWeeklyID() {
    auto lock = co_await m_weeklyID.lock();
    co_return *lock;
}

arc::Future<std::optional<int>> MentionManager::getEventID() {
    auto lock = co_await m_eventID.lock();
    co_return *lock;
}

void MentionManager::disableDailyID() {
    async::spawn(
        m_dailyID.lock(),
        [](auto lock) {
            *lock = std::nullopt;
        }
    );
}

void MentionManager::disableWeeklyID() {
    async::spawn(
        m_weeklyID.lock(),
        [](auto lock) {
            *lock = std::nullopt;
        }
    );
}

void MentionManager::disableEventID() {
    async::spawn(
        m_eventID.lock(),
        [](auto lock) {
            *lock = std::nullopt;
        }
    );
}

arc::Future<> MentionManager::commentWatcher() {
    while (true) {
        // ugly as heck but whatever
        std::vector<int> levelIDs;
        {
            auto lock = co_await m_dailyID.lock();
            if (*lock) levelIDs.push_back(**lock);
        }
        {
            auto lock = co_await m_weeklyID.lock();
            if (*lock) levelIDs.push_back(**lock);
        }
        {
            auto lock = co_await m_eventID.lock();
            if (*lock) levelIDs.push_back(**lock);
        }
        {
            auto lock = co_await m_customIDs.lock();
            levelIDs.insert(levelIDs.end(), lock->begin(), lock->end());
        }

        if (levelIDs.empty()) {
            log::error("No IDs were found");
            co_await xblazeapi::sleepSecs(1);
            continue;
        }

        for (const auto& levelID : levelIDs) {
            co_await xblazeapi::sleepSecs(Mod::get()->getSettingValue<int64_t>("refresh-rate"));
            if (!Mod::get()->getSettingValue<bool>("enabled")) {
                continue;
            }

            updateAliases();

            auto res = co_await xblazeapi::requestGDServers("getGJComments21.php", fmt::format(
                "levelID={}&page=0&secret={}",
                levelID, xblazeapi::SECRET
            ));
            if (res.isErr()) {
                // Verify it's not an internet issue
                if (!co_await xblazeapi::doWeHaveInternet()) {
                    log::error("No internet connection!");
                    notifyError("CommentMentions: No internet connection!\nPlease verify your internet connection");
                    this->pollUntilWeHaveInternet();
                    co_return;
                }

                std::string msg = fmt::format("CommentMentions: Failed to fetch comments: {}", res.unwrapErr());
                log::error("{}", msg);
                notifyError(msg);
                continue;
            }
            log::debug("{}", res.unwrap());

            // Split comment objects
            auto comments = string::split(res.unwrap(), "|");
            for (const auto& comment : comments) {
                auto obj = formatCommentObj(comment); // Format object

                log::debug("Encoded: {}", obj.comment["comment"]);

                auto s = base64::decodeString(obj.comment["comment"], base64::Base64Variant::Url);
                if (s.isErr()) {
                    log::error("Could not decode comment: {}", s.unwrapErr());
                    continue;
                }
                std::string string = std::move(s).unwrap();

                log::debug("Decoded: {}", string);

                if (containsMention(string)) {
                    // The sea of checks
                    if (isPrevious(obj)) continue;
                    if (Mod::get()->getSettingValue<bool>("ignore-self") && isSelfMention(obj.author["accountID"]))
                        continue;
                    if (isBlacklisted(obj.author["userName"])) continue;
                    if (isCommentInappropriate(string)) {
                        log::info("Inappropriate comment: {}", string);
                        continue;
                    }

                    obj.comment["comment"] = std::move(string);
                    log::info("Queued mention by {}: {}", obj.author["userName"], obj.comment["comment"]);
                    storePrevious(obj);
                    m_mentions.push_back(obj);
                }
            }

            // Skip if playing/on editor
            if (!Mod::get()->getSettingValue<bool>("show-while-playing") && PlayLayer::get()) continue;
            if (!Mod::get()->getSettingValue<bool>("show-on-editor") && LevelEditorLayer::get()) continue;

            if (!m_mentions.empty()) {
                if (m_mentions.size() > Mod::get()->getSettingValue<int64_t>("max-notifications")) {
                    onMentionCompressed(m_mentions.size());
                } else {
                    for (const auto& mention : m_mentions) {
                        onMention(mention);
                    }
                }
                m_mentions.clear();
            }
        }
    }
}

void MentionManager::onMention(const CommentObject& obj) {
    geode::queueInMainThread([this, obj] {
        auto usrIt = obj.author.find("userName");
        const std::string username = usrIt == obj.author.end() ?
            "Someone" : usrIt->second;

        auto commentIt = obj.comment.find("comment");
        const std::string comment = commentIt == obj.author.end() ?
            "" : commentIt->second;

        showNotification(fmt::format("{} mentioned you!", username), comment);
    });
}

void MentionManager::onMentionCompressed(int amount) {
    geode::queueInMainThread([this, amount] {
        const std::string msg = random::chance(.1f) ? "Never gonna give you up!" : "Check them out!";
        showNotification(fmt::format("{} new mentions!", amount), msg);
    });
}

void MentionManager::showNotification(const std::string& title, const std::string& msg) {
    AchievementNotifier::sharedState()->notifyAchievement(
        title.c_str(),
        msg.c_str(),
        "accountBtn_pendingRequest_001.png",
        true
    );
}

bool MentionManager::containsMention(const std::string& str) {
    for (const auto& tag : m_aliases) {
        if (string::contains(string::toLower(str), tag)) { 
            return true; 
        }
    } 
    return false;
}

bool MentionManager::isSelfMention(const std::string& str) {
    int ownAccID = GJAccountManager::sharedState()->m_accountID;
    auto otherAccID = utils::numFromString<int>(str);
    if (otherAccID.isErr()) {
        log::debug("Could not convert {} to int", str);
        return false;
    }
    return ownAccID == otherAccID.unwrap();
}

void MentionManager::updateAliases() {
    m_aliases.clear();

    std::vector<std::string> val;
    if (Mod::get()->getSettingValue<bool>("enable-everyone")) {
        m_aliases.push_back("@everyone");
    }

    auto setting = getListSetting("aliases");
    for (const auto& alias : setting) {
        if (string::contains(alias, "everyone")) continue; // ignore any @everyone's directly in the aliases setting
        m_aliases.push_back(alias);
    }
}

bool MentionManager::isPrevious(const CommentObject& obj) {
    auto ownMessageID = obj.comment.find("messageID");
    if (ownMessageID == obj.comment.end()) {
        notifyError("CommentMentions: An unexpected issue occured\nPlease report this issue to the developer (include the game logs)");

        log::error("Could not find 'messageID' in mention (THIS SHOULD BE UNREACHABLE)");
        log::info("PLEASE REPORT THIS BUG");
        return false;
    }

    for (auto& mention : m_previousMentions) {
        auto messageID = mention.comment["messageID"];
        if (messageID == ownMessageID->second) {
            log::debug("Mention under messageID {} was previously detected, skipping", ownMessageID->second);
            return true; 
        }
    }
    return false;
}

void MentionManager::storePrevious(const CommentObject& obj) {
    m_previousMentions.push_back(obj);
    if (m_previousMentions.size() > 20) {
        m_previousMentions.pop_front();
    }
}

void MentionManager::pollUntilWeHaveInternet() {
    m_watcher.spawn(
        "MentionManager::pollUntilWeHaveInternet",
        [] -> arc::Future<> {
            while (true) {
                if (co_await xblazeapi::doWeHaveInternet()) break;
                co_await xblazeapi::sleepSecs(3);
            }
        },
        [this] {
            Notification::create("CommentMentions: Back online ;)", NotificationIcon::Success)->show();
            this->start();
        }
    );
}

inline bool MentionManager::isCommentInappropriate(const std::string& comment) {
    return Mod::get()->getSettingValue<bool>("hide-inapropriate-comments") && isInapropriate(comment);
}

bool MentionManager::isBlacklisted(const std::string& username) {
    auto blacklist = getBlacklistedAccounts();
    auto usernameLower = string::toLower(username);

    for (const auto& blacklistedUser : blacklist) {
        if (usernameLower == string::toLower(blacklistedUser)) {
            log::info("User '{}' is blacklisted, skipping...", username);
            return true;
        }
    }
    return false;
}

std::vector<std::string> MentionManager::getBlacklistedAccounts() {
    return getListSetting("user-blacklist");
}

CommentObject MentionManager::formatCommentObj(const std::string& str) {
    auto split = string::split(str, ":");
    // Commented out in case I need them again
    // log::debug("{}", split[0]);
    // log::debug("{}", split[1]);

    CommentObject ret;
    ret.comment = formatKV(split[0], {
        { "1", "levelID" },
        { "2", "comment" },
        { "3", "authorPlayerID" },
        { "4", "likes" },
        { "5", "dislikes" },
        { "6", "messageID" },
        { "7", "spam" },
        { "8", "authorAccountID" },
        { "9", "age" },
        { "10", "percent" },
        { "11", "modBadge" },
        { "12", "moderatorChatColor" },
    }, "~");
    ret.author = formatKV(split[1], {
        { "1", "userName" },
        { "9", "icon" },
        { "10", "playerColor" },
        { "11", "playerColor2" },
        { "14", "iconType" },
        { "15", "glow" },
        { "16", "accountID" },
    }, "~");

    return ret;
}