#include "comments.hpp"

#include <arc/future/Future.hpp>
#include <arc/time/Sleep.hpp>
#include <asp/time/Duration.hpp>
#include <core/utils.hpp>
#include <core/history/history.hpp>
// #include <core/notifier/notifier.hpp>

#include <Geode/Geode.hpp>
#include <Geode/utils/base64.hpp>
#include <Geode/utils/async.hpp>
#include <Geode/utils/string.hpp>
#include <Geode/utils/Task.hpp>
#include <Geode/utils/web.hpp>

#include <chrono>
#include <string>
#include <unordered_map>
#include <vector>

using namespace geode::prelude;
using namespace geode::utils;

namespace comments {
    CommentListener::CommentListener(int levelID) :
        m_levelID(levelID)
    {};

    void CommentListener::start() {
        m_listener.spawn(
            "Comment Listener",
            commentEval(),
            [] {}
        );
    }

    void CommentListener::stop() {
        m_listener.cancel();
    }

    arc::Future<> CommentListener::commentEval() {
        while (true) {
            std::vector<CMUtils::CommentObject> foundComments;

            auto req = web::WebRequest()
                .userAgent("")
                .timeout(std::chrono::seconds(10))
                .bodyString("levelID=" + utils::numToString(m_levelID) + "&page=0&secret=" + CMUtils::SECRET);

            auto res = co_await req.post(CMUtils::BOOMLINGS + "getGJComments21.php");
            if (res.ok() && CMUtils::stringIsOk(res.string())) {
                auto comments = string::split(res.string().unwrap(), "|");
                for (const auto& comment : comments) {
                    auto obj = CMUtils::formatCommentObj(comment);
                    if (containsMention(obj.comment["comment"])) {
                        foundComments.push_back(obj);
                    }
                }
            } else {
                log::error("Failed to fetch comments for ID '{}'", m_levelID);
                log::info("Status code: {}", res.code());
                log::info("Response: '{}'", res.string().unwrapOr("N/A"));
            }

            if (!foundComments.empty()) {
                for (const auto& comment : foundComments) {
                    auto username = comment.author.find("userName");
                    auto accountID = comment.author.find("accountID");
                    auto msg = comment.author.find("comment");

                    onMention(
                        username != comment.author.end() ? username->second : "User",
                        accountID != comment.author.end() ? accountID->second : "-1",
                        msg != comment.comment.end() ? msg->second : "N/A"
                    );
                }
            }

            co_await arc::sleep(asp::Duration::fromSecs(10));
        }
    }

    bool CommentListener::containsMention(const std::string& str) {
        std::vector<std::string> tags{ "xblazegmd", "xblaze", "blaze" }; // Hardcoded for testing
        auto lower = string::toLower(str);
        for (const auto& tag : tags) {
            if (string::contains(lower, tag)) return true;
        }
        return false;
    }

    void CommentListener::onMention(const std::string& user, const std::string accountID, const std::string& msg) {
        log::info("Mention from @{}: '{}'", user, msg);
    }

    // ListenerTask CommentListener::startListener() {
    //     while (true) {
    //         auto mentions = co_await evalComments();

    //         if (!mentions.empty()) {
    //             for (auto mention : mentions) {
    //                 auto commentDecodedRes = base64::decode(mention["commentStr"]["comment"], base64::Base64Variant::Url);
    //                 if (commentDecodedRes.isErr()) {
    //                     log::error("Could not decode comment '{}': {}", mention["commentStr"]["comment"], commentDecodedRes.unwrapErr());
    //                     continue;
    //                 }

    //                 auto bytes = commentDecodedRes.unwrap();
    //                 std::string commentDecoded(bytes.begin(), bytes.end());

    //                 std::unordered_map<std::string, std::string> mentionData{
    //                     { "comment", commentDecoded },
    //                     { "messageID", mention["commentStr"]["messageID"] },
    //                     { "authorUsername", mention["authorStr"]["username"] },
    //                     { "authorAccID", mention["authorStr"]["accID"] },
    //                     { "authorIcon", mention["authorStr"]["icon"] },
    //                     { "authorColorA", mention["authorStr"]["colorA"] },
    //                     { "authorColorB", mention["authorStr"]["colorB"] },
    //                     { "authorIconType", mention["authorStr"]["iconType"] },
    //                     { "authorGlow", mention["authorStr"]["glow"] }
    //                 };

    //                 onMention(mention["authorStr"]["username"], commentDecoded, mentionData);
    //             }
    //         }

    //         co_await coro::sleep(10);
    //     }
    // }

    // EvalTask CommentListener::evalComments() {
    //     return EvalTask::runWithCallback([this](auto finish, auto prog, auto isCancelled) {
    //         std::string params = "levelID=" + std::to_string(this->m_levelID) + "&page=0&secret=" + CMUtils::SECRET;

    //         auto req = web::WebRequest();
    //         req.userAgent("");
    //         req.timeout(std::chrono::seconds(10));
    //         req.bodyString(params);

    //         req.post(CMUtils::BOOMLINGS + "getGJComments21.php")
    //             .listen([this, finish](web::WebResponse *res) {
    //                 if (res && res->ok() && res->string().isOk()) {
    //                     auto comments = string::split(res->string().unwrap(), "|");
    //                     std::vector<std::unordered_map<std::string, formatReq::StrMap>> foundComments;

    //                     for (std::string comment : comments) {
    //                         auto commentObj = formatReq::formatCommentObj(comment);
    //                         if (this->containsMention(commentObj["commentStr"]["comment"])) {
    //                             foundComments.push_back(commentObj);
    //                         }
    //                     }

    //                     finish(foundComments);
    //                 } else {
    //                     log::error("Could not fetch comments ({}, response: {})", res->code(), res->string().unwrapOr("..."));
    //                     std::vector<std::unordered_map<std::string, formatReq::StrMap>> emptyLol;
    //                     finish(emptyLol);
    //                 }
    //             });
    //     }, "evalComments");
    // }

    // bool CommentListener::containsMention(std::string str) {
    //     std::vector<std::string> tags = getTags();

    //     auto commentRes = base64::decode(str, base64::Base64Variant::Url);
    //     if (commentRes.isErr()) {
    //         log::error("Could not decode comment '{}': {}", str, commentRes.unwrapErr());
    //         return false;
    //     }

    //     auto bytes = commentRes.unwrap();
    //     std::string comment(bytes.begin(), bytes.end());

    //     std::string commentLower = string::toLower(comment);
    //     for (const auto& tag : tags) {
    //         if (CMUtils::contains(commentLower, tag)) {
    //             return true;
    //         }
    //     }
    //     return false;
    // }

    // std::vector<std::string> CommentListener::getTags() {
    //     auto tags = Mod::get()->getSettingValue<std::string>("tags");
    //     auto parts = string::split(tags, ",");

    //     for (auto& part : parts) {
    //         part = string::trim(part);
    //     }
    //     return parts;
    // }

    // void CommentListener::onMention(std::string user, std::string msg, std::unordered_map<std::string, std::string> data) {
    //     if (history::mentionExists(data)) return;
    //     history::updateHistory(data);

	//     log::info("Mention from @{}, '{}'", user, msg);
    //     m_notifier.notify(
    //         user + " mentioned you",
    //         msg
    //     );
    // }
}