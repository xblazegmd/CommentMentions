#include "MentionManager.hpp"

#include <utils.hpp>
#include <filtering.hpp>
#include <CommentObject.hpp>

#include <Geode/Geode.hpp>
#include <Geode/ui/Notification.hpp>
#include <Geode/utils/base64.hpp>
#include <Geode/utils/web.hpp>
#include <Geode/utils/random.hpp>
#include <Geode/utils/string.hpp>

#include <arc/prelude.hpp>
#include <fmt/format.h>
#include <ranges>
#include <regex>
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
        log::error("CommentMentions: Could not get daily level's ID: {}", levelID.unwrapErr());
        co_return;
    }
    auto lock = co_await m_dailyID.lock();
    *lock = std::move(levelID).unwrap();
}

arc::Future<> MentionManager::fetchWeeklyID() {
    auto levelID = co_await getSpecialID(LevelType::Weekly);
    if (levelID.isErr()) {
        log::error("CommentMentions: Could not get weekly level's ID: {}", levelID.unwrapErr());
        co_return;
    }
    auto lock = co_await m_weeklyID.lock();
    *lock = std::move(levelID).unwrap();
}

arc::Future<> MentionManager::fetchEventID() {
    auto levelID = co_await getSpecialID(LevelType::Event);
    if (levelID.isErr()) {
        log::error("CommentMentions: Could not get event level's ID: {}", levelID.unwrapErr());
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
        auto idNum = stoicm(string::trim(id));
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

std::deque<CommentObject> MentionManager::getPreviousMentions() const {
    return m_previousMentions;
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

            // Update aliases
            std::vector<std::string> aliases;
            if (Mod::get()->getSettingValue<bool>("enable-everyone")) {
                aliases.push_back("@everyone");
            }

            auto setting = getListSetting("aliases");
            for (const auto& alias : setting) {
                if (string::contains(alias, "everyone")) continue; // ignore any @everyone's directly in the aliases setting
                aliases.push_back(alias);
            }

            m_aliasRegex = std::regex(
                fmt::format(
                    "\\b{}(?:{})\\b",
                    Mod::get()->getSettingValue<bool>("require-at") ? "@" : "",
                    string::join(aliases, "|")
                ),
                Mod::get()->getSettingValue<bool>("case-sensitive") ?
                    std::regex::optimize : std::regex::icase | std::regex::optimize
            );

            // Get comments from the level
            auto res = co_await xblazeapi::requestGDServers("getGJComments21.php", xblazeapi::buildBodyString({
                { "levelID", utils::numToString(levelID) },
                { "page", "0" },
                { "secret", xblazeapi::SECRET }
            }));
            if (res.isErr()) {
                // Verify it's not an internet issue
                bool internet;
                if (Mod::get()->getSettingValue<bool>("internal-internet-check")) {
                    internet = co_await async::waitForMainThread([] {
                        return GameToolbox::doWeHaveInternet();
                    });
                } else {
                    internet = co_await xblazeapi::doWeHaveInternet();
                }

                if (!internet) {
                    log::error("No internet connection!");
                    co_await pauseUntilWeHaveInternet();
                    log::info("Back online ;)");
                    continue;
                }

                // Maybe rob's a troll and rate limited us
                auto err = res.unwrapErr();
                if (err == 429) {
                    log::error("Rate limited :(");
                    log::info("Please restart your game once the rate limit ends");
                    co_return; // Exit early since there's no point in doing anything atp
                }

                log::error("Failed to fetch comments: {}", res.unwrapErr());
                continue;
            }
            log::debug("{}", res.unwrap());

            // Split comment objects
            auto comments = string::split(res.unwrap(), "|");
            for (const auto& comment : comments) {
                auto obj = CommentObject::fromString(comment); // Format object

                log::debug("Encoded: {}", obj.commentt);

                auto s = base64::decodeString(obj.commentt, base64::Base64Variant::Url);
                if (s.isErr()) {
                    log::error("Could not decode comment: {}", s.unwrapErr());
                    continue;
                }
                std::string string = std::move(s).unwrap();

                log::debug("Decoded: {}", string);

                if (containsMention(string)) {
                    // The sea of checks
                    if (isPrevious(obj)) continue;
                    if (Mod::get()->getSettingValue<bool>("ignore-self") && isSelfMention(obj.accountID))
                        continue;
                    if (isBlacklisted(obj.username)) continue;
                    if (isCommentInappropriate(string)) {
                        log::info("Inappropriate comment: {}", string);
                        continue;
                    }

                    obj.commentt = std::move(string);
                    obj.levelID = levelID;
                    log::info("Queued mention by {}: {}", obj.username, obj.commentt);
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
        showNotification(fmt::format("{} mentioned you!", obj.username), obj.commentt);
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

inline bool MentionManager::containsMention(const std::string& str) {
    return std::regex_search(str, m_aliasRegex);
}

inline bool MentionManager::isSelfMention(int accountID) {
    return GJAccountManager::sharedState()->m_accountID == accountID;
}

bool MentionManager::isPrevious(const CommentObject& obj) {
    for (auto& mention : m_previousMentions) {
        auto messageID = mention.messageID;
        if (messageID == obj.messageID) {
            log::debug("Mention under message ID {} was previously detected, skipping", messageID);
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

inline bool MentionManager::isCommentInappropriate(const std::string& comment) {
    return Mod::get()->getSettingValue<bool>("hide-inappropriate-comments") && isInappropriate(comment);
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