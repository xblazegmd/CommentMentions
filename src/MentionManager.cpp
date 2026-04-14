#include "MentionManager.hpp"

#include <arc/prelude.hpp>
#include <chrono>
#include <cstdint>
#include <utils.hpp>

#include <Geode/Geode.hpp>
#include <Geode/utils/base64.hpp>
#include <Geode/utils/web.hpp>
#include <Geode/utils/string.hpp>

#include <string>
#include <vector>

#include <xblazegmd.geode-api/include/XblazeAPI.hpp>

using namespace geode::prelude;

MentionManager* MentionManager::get() {
    static MentionManager* instance = new MentionManager();
    return instance;
}

arc::Future<> MentionManager::setLevelIDs(std::vector<int> levelIDs) {
    auto lock = co_await m_levelIDs.lock();
    *lock = std::move(levelIDs);
}

void MentionManager::start() {
    m_watcher.spawn(
        "commentWatcher",
        commentWatcher(),
        [] {}
    );
}

arc::Future<> MentionManager::commentWatcher() {
    while (true) {
        auto lock = co_await m_levelIDs.lock();
        for (const auto& levelID : *lock) {
            co_await xblazeapi::sleepSecs(Mod::get()->getSettingValue<int64_t>("refresh-rate"));

            auto now = std::chrono::steady_clock::now();
            if (now >= m_nextInternetCheck) {
                m_doWeHaveInternet = co_await doWeHaveInternet();
                if (!m_doWeHaveInternet) notifyError("CommentMentions: No internet connection!");
                m_nextInternetCheck = now + std::chrono::seconds(120);
            }

            if (!m_doWeHaveInternet) {
                continue;
            }

            auto req = co_await xblazeapi::requestGDServers("getGJComments21.php", fmt::format(
                "levelID={}&page=0&secret={}",
                levelID, xblazeapi::SECRET
            ));

            if (req.isErr()) {
                log::error("Failed to request endpoint 'getGJComments21.php': {}", req.unwrapErr());
                notifyError(fmt::format("CommentMentions: Failed to fetch comments: {}", req.unwrapErr()));
                continue;
            }

            auto res = req.unwrap();
            auto comments = string::split(res, "|");
            for (const auto& comment : comments) {
                auto obj = formatCommentObj(comment);

                log::debug("Encoded: {}", obj.comment["comment"]);
                auto s = base64::decode(obj.comment["comment"], base64::Base64Variant::Url);
                if (s.isErr()) {
                    log::error("Could not decode comment: {}", s.unwrapErr());
                    continue;
                }
                std::string string(s.unwrap().begin(), s.unwrap().end());

                log::debug("Decoded: {}", string);

                if (containsMention(string)) {
                    if (isPrevious(obj)) continue;
                    if (Mod::get()->getSettingValue<bool>("ignore-self") && isSelfMention(obj.author["accountID"]))
                        continue;

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
                for (const auto& mention : m_mentions) {
                    geode::queueInMainThread([this, mention] {
                        onMention(mention);
                    });
                }
                m_mentions.clear();
            }
        }
    }
}

void MentionManager::onMention(const CommentObject& obj) {
    auto username = obj.author.find("userName");
    if (username == obj.author.end()) {
        notifyError("CommentMention: An unexpected issue occurred");
        notifyError("Please report this issue to the developer (include the game logs)");

        log::error("Could not find 'userName' in object (THIS SHOULD BE UNREACHABLE)");
        log::info("PLEASE REPORT THIS BUG");
        return;
    }
    auto comment = obj.comment.find("comment");
    if (comment == obj.author.end()) {
        notifyError("CommentMention: An unexpected issue occurred");
        notifyError("Please report this issue to the developer (include the game logs)");

        log::error("Could not find 'comment' in object (THIS SHOULD BE UNREACHABLE)");
        log::info("PLEASE REPORT THIS BUG");
        return;
    }

    AchievementNotifier::sharedState()->notifyAchievement(
        fmt::format("{} mentioned you", username->second).c_str(),
        comment->second.c_str(),
        "accountBtn_pendingRequest_001.png",
        true
    );
}

bool MentionManager::containsMention(const std::string& str) {
    auto aliases = getAliases();
    for (const auto& tag : aliases)
        if (string::contains(string::toLower(str), tag)) { 
            return true; 
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

bool MentionManager::isPrevious(const CommentObject& obj) {
    auto ownMessageID = obj.comment.find("messageID");
    if (ownMessageID == obj.comment.end()) {
        notifyError("CommentMention: An unexpected issue occurred");
        notifyError("Please report this issue to the developer (include the game logs)");

        log::error("Could not find 'messageID' in mention (THIS SHOULD BE UNREACHABLE)");
        log::info("PLEASE REPORT THIS BUG");
        return false;
    }

    for (const auto& mention : m_previousMentions) {
        auto messageID = mention.comment.find("messageID");
        if (messageID == mention.comment.end()) {
            notifyError("CommentMention: An unexpected issue occurred");
            notifyError("Please report this issue to the developer (include the game logs)");

            log::error("Could not find 'messageID' in previous mention (THIS SHOULD BE UNREACHABLE)");
            log::info("PLEASE REPORT THIS BUG");
            return false;
        }
        if (messageID->second == ownMessageID->second) { 
            log::debug("Mention under messageID {} was previously detected, skipping", ownMessageID->second);
            return true; 
        }
    }
    return false;
}

void MentionManager::storePrevious(const CommentObject& obj) {
    m_previousMentions.push_back(obj);
    if (m_previousMentions.size() > 20) {
        m_previousMentions.erase(m_previousMentions.begin()); // Pop front
    }
}

std::vector<std::string> MentionManager::getAliases() {
    auto aliases = Mod::get()->getSettingValue<std::string>("aliases");
    auto aliasesSplit = string::split(aliases, ",");
    std::vector<std::string> ret;
    for (const auto& alias : aliasesSplit) {
        ret.push_back(string::trim(alias));
    }
    return ret;
}

MentionManager::CommentObject MentionManager::formatCommentObj(const std::string& str) {
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

arc::Future<bool> MentionManager::doWeHaveInternet() {
    auto res = co_await web::WebRequest()
        .userAgent("GeometryDash/2.2081 CommentMentions/1.0.0-beta.2")
        .timeout(std::chrono::seconds(10))
        .get("http://connectivitycheck.gstatic.com/generate_204");
    co_return res.ok();
}