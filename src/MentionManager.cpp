#include "MentionManager.hpp"

#include <Geode/Geode.hpp>
#include <Geode/Result.hpp>
#include <Geode/utils/async.hpp>
#include <Geode/utils/base64.hpp>

#include <core/UserMention.hpp>
#include <utils.hpp>

#include <arc/prelude.hpp>
#include <asp/time.hpp>
#include <asp/time/detail.hpp>

using namespace geode::prelude;

MentionManager* MentionManager::get() {
    static MentionManager* instance = new MentionManager();
    return instance;
}

arc::Future<> MentionManager::init() {
    auto mod = Mod::get();

    // Load mentions
    m_mentions = std::ranges::to<std::deque<UserMention>>(
        mod->getSavedValue<std::vector<UserMention>>("mention-history")
    );

    // Load levels
    auto addSpecialLevel = [this](LevelType type) -> arc::Future<> {
        auto id = co_await getSpecialID(type);
        if (id.isOk()) {
            m_levels.push_back(id.unwrap());
        } else {
            log::error("Could not get level ID: {}", id.unwrapErr());
        }
    };

    if (mod->getSettingValue<bool>("daily-lvl")) {
        co_await addSpecialLevel(LevelType::Daily);
    }
    if (mod->getSettingValue<bool>("weekly-demon")) {
        co_await addSpecialLevel(LevelType::Weekly);
    }
    if (mod->getSettingValue<bool>("event-lvl")) {
        co_await addSpecialLevel(LevelType::Event);
    }

    if (mod->getSettingValue<bool>("use-custom-ids")) {
        auto ids = mod->getSettingValue<std::string>("custom-ids");
        auto split = string::split(ids, ",");
        m_levels.reserve(split.size());

        for (const auto& id : split) {
            auto num = stoicm(string::trim(id));
            if (num.isErr()) {
                log::error("Could not get level ID: {}", num.unwrapErr());
                continue;
            }
            m_levels.push_back(num.unwrap());
        }
    }

    // Load group chats
    auto accMan = GJAccountManager::get();
    auto res = co_await requestGDServersCM("getGJThreads.php", buildBodyStringCM({
        { "accountID", utils::numToString(accMan->m_accountID) },
        { "gjp2", accMan->m_GJP2 },
        { "secret", SECRET }
    }));

    if (res.isErr()) {
        log::error("Failed to get group chats: {}", res.unwrapErr());
        co_return;
    }

    auto threads = string::split(res.unwrap(), "|");
    for (const auto& thread : threads) {
        auto split = string::split(thread, ":");
        if (split[12] == "1") {
            log::debug("Thread '{}' is not a group chat, skipping...", split[0]);
            continue;
        }

        auto id = stoicm(split[0]);
        if (id.isErr()) {
            log::error("Could not get thread ID: {}", id.unwrapErr());
            continue;
        }
        m_groupChats.push_back(id.unwrap());
    }
}

void MentionManager::start() {
    m_pollTask.spawn(
        "User Mentions Mention Manager",
        poll(),
        [] {}
    );
}

void MentionManager::save() {
    log::info("Saving mentions...");
    Mod::get()->setSavedValue("mention-history", std::ranges::to<std::vector<UserMention>>(m_mentions));
    log::info("Sucessfully saved mentions!");
}

arc::Future<> MentionManager::poll() {
    while (true) {
        geode::queueInMainThread([this] {
            pushNotification("Checking", "Looking for mentions. Next check in 60 seconds");
        });

        if (!m_levels.empty()) {
            auto res = co_await searchLevelComments();
            if (res.isErr()) {
                log::error("{}", res.unwrapErr());
            }
        }

        if (!m_groupChats.empty()) {
            auto res = co_await searchGroupChatMessages();
            if (res.isErr()) {
                log::error("{}", res.unwrapErr());
            }
        }

        if (!Mod::get()->getSettingValue<bool>("show-while-playing") && PlayLayer::get()) continue;
        if (!Mod::get()->getSettingValue<bool>("show-on-editor") && LevelEditorLayer::get()) continue;

        if (!m_queue.empty()) {
            if (m_queue.size() > Mod::get()->getSettingValue<int>("max-notifications")) {
                notifyMentions(m_queue.size());
            } else {
                for (const auto& mention : m_queue) {
                    notifyMention(mention);
                }
            }
            m_queue.clear();
        }

        co_await arc::sleep(asp::Duration::fromSecs(60));
    }
}

arc::Future<geode::Result<>> MentionManager::searchLevelComments() {
    int levelID = nextLevel();

    auto res = co_await requestGDServersCM("getGJComments21.php", buildBodyStringCM({
        { "levelID", utils::numToString(levelID) },
        { "page", "0" },
        { "secret", SECRET }
    }));
    if (res.isErr())
        co_return Err("Could not get comments for level ID '{}': {}", levelID, res.unwrapErr());

    auto comments = string::split(res.unwrap(), "|");
    for (const auto& comment : comments) {
        auto split = string::split(comment, ":");
        auto commentStr = formatResponseCM(split[0], "~");
        auto authorStr = formatResponseCM(split[1], "~");

        auto str = base64::decodeString(commentStr["2"]);
        if (str.isErr()) {
            log::error("Could not decode base64: {}", str.unwrapErr());
            continue;
        }

        if (containsMention(str.unwrap())) {
            auto mention = UserMention{
                MentionType::Comment,
                str.unwrap(),
                authorStr["1"],
                stoicm(authorStr["16"], -1),
                stoicm(commentStr["6"], -1),
                asp::Instant::fromRawNanos(
                    asp::time_detail::secs_to_nanos(stoicm(commentStr["15"], 0))
                )
            };
            if (mentionExists(mention)) continue;
            if (Mod::get()->getSettingValue<bool>("ignore-self") && isOwnMention(mention)) continue;

            addMention(mention);
            m_queue.push_back(mention);
        }
    }

    co_return Ok();
}

arc::Future<geode::Result<>> MentionManager::searchGroupChatMessages() {
    int threadID = nextGroupChat();

    auto accMan = GJAccountManager::get();
    auto res = co_await requestGDServersCM("getGJThreadMessages.php", buildBodyStringCM({
        { "accountID", utils::numToString(accMan->m_accountID) },
        { "gjp2", accMan->m_GJP2 },
        { "secret", SECRET },
        { "threadID", utils::numToString(threadID) }
    }));
    if (res.isErr())
        co_return Err("Could not get messages for thread ID '{}': {}", threadID, res.unwrapErr());

    auto splitMetadata = string::split(res.unwrap(), "#");
    auto messages = string::split(splitMetadata[0], "|");

    for (const auto& message : messages) {
        auto split = string::split(message, ":");

        // System messages have "accountID" (2nd list item) set to 0
        // We don't care about those so we skip them
        if (split[1] == "0") {
            log::debug("Found system message, skipping...");
            continue;
        }

        auto str = base64::decodeString(split[2]);
        if (str.isErr()) {
            log::error("Could not decode base64: {}", str.unwrapErr());
            continue;
        }

        if (containsMention(str.unwrap())) {
            int accountID = stoicm(split[1], -1);

            // Find username
            auto users = string::split(splitMetadata[1], "|");
            std::string username = "user";
            for (const auto& user : users) {
                auto data = formatResponseCM(user, ":");
                if (stoicm(data["16"], 0) == accountID) {
                    username = data["1"];
                    break;
                }
            }

            auto mention = UserMention{
                MentionType::DM,
                str.unwrap(),
                username,
                accountID,
                stoicm(split[0], -1),
                asp::Instant::fromRawNanos(
                    asp::time_detail::secs_to_nanos(stoicm(split[4], 0))
                )
            };
            if (mentionExists(mention)) continue;
            if (Mod::get()->getSettingValue<bool>("ignore-self") && isOwnMention(mention)) continue;

            addMention(mention);
            m_queue.push_back(mention);
        }
    }
    
    co_return Ok();
}

int MentionManager::nextLevel() {
    int current = m_levelIdx;
    m_levelIdx++;
    
    if (m_levelIdx >= m_levels.size())
        m_levelIdx = 0;
    
    return m_levels[current];
}

int MentionManager::nextGroupChat() {
    int current = m_groupChatIdx;
    m_groupChatIdx++;
    
    if (m_groupChatIdx >= m_groupChats.size())
        m_groupChatIdx = 0;
    
    return m_groupChats[current];
}

inline bool MentionManager::containsMention(const std::string& message) {
    return string::contains(message, "@xblaze");
}

inline bool MentionManager::isOwnMention(const UserMention& mention) {
    return mention.accountID == GJAccountManager::get()->m_accountID;
}

bool MentionManager::mentionExists(const UserMention& other) {
    for (const auto& mention : m_mentions) {
        if (mention == other) {
            log::info("Mention under message ID '{}' was previously detected, skipping...", mention.messageID);
            return true;
        }
    }
    return false;
}

void MentionManager::addMention(const UserMention& mention) {
    m_mentions.push_front(mention);
    if (m_mentions.size() > 20) {
        m_mentions.pop_back();
    }
}

void MentionManager::notifyMention(const UserMention& mention) {
    geode::queueInMainThread([this, mention] {
        pushNotification(fmt::format("@{} mentioned you!", mention.username), mention.message);
    });
}

void MentionManager::notifyMentions(int mentions) {
    geode::queueInMainThread([this, mentions] {
        const std::string msg = random::chance(.1f) ? "Never gonna give you up!" : "Check them out!";
        pushNotification(fmt::format("{} new mentions!", mentions), msg);
    });
}

void MentionManager::pushNotification(const std::string& title, const std::string& message) {
    AchievementNotifier::sharedState()->notifyAchievement(
        title.c_str(),
        message.c_str(),
        "particle_299_001.png",
        true
    );
}
