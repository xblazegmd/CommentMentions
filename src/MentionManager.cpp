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

arc::Future<> MentionManager::getLevels() {
    auto mod = Mod::get();

    auto addSpecialLevel = [this](LevelType type) -> arc::Future<> {
        auto id = co_await getSpecialID(type);
        if (id.isOk()) {
            m_targets.emplace_back(TargetType::Level, id.unwrap());
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
        m_targets.reserve(split.size());

        for (const auto& id : split) {
            auto num = stoicm(string::trim(id));
            if (num.isErr()) {
                log::error("Could not get level ID: {}", num.unwrapErr());
                continue;
            }

            m_targets.emplace_back(TargetType::Level, num.unwrap());
        }
    }
}

arc::Future<> MentionManager::getGroupChats() {
    auto accountID = co_await async::waitForMainThread([] {
        return GJAccountManager::get()->m_accountID;
    });
    if (!accountID) {
        log::error("Could not get group chats (failed to get account ID)");
        co_return;
    }

    auto gjp2 = co_await async::waitForMainThread([] {
        return GJAccountManager::get()->m_GJP2;
    });
    if (!gjp2) {
        log::error("Could not get group chats (failed to get gjp2)");
        co_return;
    }

    auto res = co_await requestGDServersCM("getGJThreads.php", buildBodyStringCM({
        { "accountID", utils::numToString(*accountID) },
        { "gjp2", *gjp2 },
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
        m_targets.emplace_back(TargetType::GroupChat, id.unwrap());
    }
}

void MentionManager::start() {
    if (m_targets.empty()) {
        log::error("No targets were found");
        return;
    }

    m_pollTask.spawn(
        "User Mentions Mention Manager",
        poll(),
        [] {}
    );
}

void MentionManager::stop() {
    m_pollTask.cancel();
}

arc::Future<> MentionManager::poll() {
    while (true) {
        for (const auto& target : m_targets) {
            co_await arc::sleep(asp::Duration::fromSecs(10));

            auto res = target.type == TargetType::Level ?
                co_await searchLevelComments(target.id) :
                co_await searchGroupChat(target.id);

            if (res.isErr()) {
                log::error("{}", res.unwrapErr());
                geode::queueInMainThread([res] {
                    Notification::create(fmt::format("{}", res.unwrapErr()))->show();
                });
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
        }
    }
}

arc::Future<geode::Result<>> MentionManager::searchLevelComments(int levelID) {
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
                authorStr["0"],
                stoicm(authorStr["16"], -1),
                stoicm(commentStr["6"], -1),
                asp::Instant::fromRawNanos(
                    asp::time_detail::secs_to_nanos(stoicm(commentStr["15"], 0))
                )
            };

            if (mentionExists(mention)) continue;

            addMention(mention);
            m_queue.push_back(mention);
        }
    }

    co_return Ok();
}

arc::Future<geode::Result<>> MentionManager::searchGroupChat(int threadID) {
    // There has to be a better way to get this
    auto accountID = co_await async::waitForMainThread([] {
        return GJAccountManager::get()->m_accountID;
    });
    if (!accountID) {
        co_return Err("Could not get group chats (failed to get account ID)");
    }

    auto gjp2 = co_await async::waitForMainThread([] {
        return GJAccountManager::get()->m_GJP2;
    });
    if (!gjp2) {
        co_return Err("Could not get group chats (failed to get gjp2)");
    }

    auto res = co_await requestGDServersCM("getGJThreadMessages.php", buildBodyStringCM({
        { "accountID", utils::numToString(*accountID) },
        { "gjp2", *gjp2 },
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
            auto mention = UserMention{
                MentionType::DM,
                str.unwrap(),
                "[USER]", // TODO: Get actual username
                stoicm(split[1], -1),
                stoicm(split[0], -1),
                asp::Instant::fromRawNanos(
                    asp::time_detail::secs_to_nanos(stoicm(split[4], 0))
                )
            };

            if (mentionExists(mention)) continue;

            addMention(mention);
            m_queue.push_back(mention);
        }
    }
    
    co_return Ok();
}

bool MentionManager::containsMention(const std::string& message) {
    return string::contains(message, "@xblaze");
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
