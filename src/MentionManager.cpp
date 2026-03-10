#include "MentionManager.hpp"

#include <arc/prelude.hpp>
#include <utils.hpp>
#include <Geode/Geode.hpp>
#include <Geode/utils/base64.hpp>
#include <Geode/utils/web.hpp>
#include <Geode/utils/string.hpp>
#include <chrono>
#include <string>
#include <vector>

using namespace geode::prelude;

MentionManager::MentionManager(std::vector<int> targets) : m_targets(targets) {};

void MentionManager::start() {
    m_watcher.spawn(
        "MentionManager::mentionTracker",
        commentWatcher(),
        [] {}
    );
}

arc::Future<> MentionManager::commentWatcher() {
    while (true) {
        for (const auto& levelID : m_targets) {
            co_await arc::sleep(asp::Duration::fromSecs(
                Mod::get()->getSettingValue<int64_t>("refresh-rate")
            ));

            auto req = web::WebRequest()
                .userAgent("")
                .timeout(std::chrono::seconds(10))
                .bodyString("levelID=" + utils::numToString(levelID) + "&page=0&secret=" + SECRET);

            auto res = co_await req.post(BOOMLINGS + "getGJComments21.php");

            if (!res.ok() || res.string().isErr()) {
                log::error("Request failed: (status code: {})", res.code());
                continue;
            }

            std::string resStr = res.string().unwrap();
            auto resStrNum = utils::numFromString<int>(resStr);

            if (resStrNum.isOk() && resStrNum.unwrap() < 0) {
                log::error("Request failed: {}", resStr);
                continue;
            }

            auto comments = string::split(resStr, "|");
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

            if (PlayLayer::get()) continue; // Skip if playing

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
        log::error("Could not find 'userName' in object (THIS SHOULD BE UNREACHABLE)");
        log::info("PLEASE REPORT THIS BUG");
        return;
    }
    auto comment = obj.comment.find("comment");
    if (comment == obj.author.end()) {
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
        log::error("Could not find 'messageID' in mention (THIS SHOULD BE UNREACHABLE)");
        log::info("PLEASE REPORT THIS BUG");
        return false;
    }

    for (const auto& mention : m_previousMentions) {
        auto messageID = mention.comment.find("messageID");
        if (messageID == mention.comment.end()) {
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