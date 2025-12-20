#include "comments.hpp"

#include <core/utils.hpp>
#include <core/formatReq/formatReq.hpp>
#include <core/history/history.hpp>
#include <core/notifier/notifier.hpp>
#include <core/queue/queue.hpp>

#include <Geode/Geode.hpp>
#include <Geode/utils/base64.hpp>
#include <Geode/utils/coro.hpp>
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
        m_listenerCoro = std::get<0>(coro::spawn << startListener());
        m_running = true;
    }

    void CommentListener::stop() {
        if (m_running) {
            m_listenerCoro.cancel();
        }
    }

    ListenerTask CommentListener::startListener() {
        while (true) {
            // auto q = queue::loadQueue();
            // if (!q.empty() && !PlayLayer::get()) { // '!PlayLayer::get() since we don't wanna pop the queue while the user is playing, duh
            //     log::info("Popping queue...");
            //     queue::clearQueue();

            //     for (const auto& mention : q) {
            //         auto qUserIt = mention.find("authorUsername");
            //         auto qMsgIt = mention.find("comment");

            //         // Fallbacks
            //         std::string qUser = qUserIt != mention.end() ? qUserIt->second : "Someone";
            //         std::string qMsg = qMsgIt != mention.end() ? qMsgIt->second : "Could not load mention but it exists, trust me";

            //         onMention(qUser, qMsg, mention);
            //     }

            //     log::info("Loaded {} mentions", q.size());
            // }

            auto mentions = co_await evalComments();

            if (!mentions.empty()) {
                for (auto mention : mentions) {
                    auto commentDecodedRes = base64::decode(mention["commentStr"]["comment"], base64::Base64Variant::Url);
                    if (commentDecodedRes.isErr()) {
                        log::error("Could not decode comment '{}': {}", mention["commentStr"]["comment"], commentDecodedRes.unwrapErr());
                        continue;
                    }

                    auto bytes = commentDecodedRes.unwrap();
                    std::string commentDecoded(bytes.begin(), bytes.end());

                    std::unordered_map<std::string, std::string> mentionData{
                        { "comment", commentDecoded },
                        { "messageID", mention["commentStr"]["messageID"] },
                        { "authorUsername", mention["authorStr"]["username"] },
                        { "authorAccID", mention["commentStr"]["authorAccID"] },
                        { "authorIcon", mention["authorStr"]["icon"] },
                        { "authorColorA", mention["authorStr"]["colorA"] },
                        { "authorColorB", mention["authorStr"]["colorB"] },
                        { "authorIconType", mention["authorStr"]["iconType"] },
                        { "authorGlow", mention["authorStr"]["glow"] }
                    };

                    // Queue mention if the user is playing a level (old, commented out)
                    // if (PlayLayer::get()) {
                    //     log::info("Queued mention from @{}, '{}'", mention["authorStr"]["username"], commentDecoded);
                    //     queue::addToQueue(mentionData);
                    //     continue;
                    // }

                    onMention(mention["authorStr"]["username"], commentDecoded, mentionData);
                }
            }

            co_await coro::sleep(10);
        }
    }

    EvalTask CommentListener::evalComments() {
        return EvalTask::runWithCallback([this](auto finish, auto prog, auto isCancelled) {
            std::string params = "levelID=" + std::to_string(this->m_levelID) + "&page=0&secret=" + CMUtils::SECRET;

            auto req = web::WebRequest();
            req.userAgent("");
            req.timeout(std::chrono::seconds(10));
            req.bodyString(params);

            req.post(CMUtils::BOOMLINGS + "getGJComments21.php")
                .listen([this, finish](web::WebResponse *res) {
                    if (res && res->ok() && res->string().isOk()) {
                        auto comments = string::split(res->string().unwrap(), "|");
                        std::vector<std::unordered_map<std::string, formatReq::StrMap>> foundComments;

                        for (std::string comment : comments) {
                            auto commentObj = formatReq::formatCommentObj(comment);
                            if (this->containsMention(commentObj["commentStr"]["comment"])) {
                                foundComments.push_back(commentObj);
                            }
                        }

                        finish(foundComments);
                    } else {
                        log::error("Could not fetch comments ({}, response: {})", res->code(), res->string().unwrapOr("..."));
                        std::vector<std::unordered_map<std::string, formatReq::StrMap>> emptyLol;
                        finish(emptyLol);
                    }
                });
        }, "evalComments");
    }

    bool CommentListener::containsMention(std::string str) {
        std::vector<std::string> tags = getTags();

        auto commentDecodedRes = base64::decode(str, base64::Base64Variant::Url);
        if (commentDecodedRes.isErr()) {
            log::error("Could not decode comment '{}': {}", str, commentDecodedRes.unwrapErr());
            return false;
        }

        auto bytes = commentDecodedRes.unwrap();
        std::string commentDecoded(bytes.begin(), bytes.end());

        std::string commentDecodedLower = string::toLower(commentDecoded);
        for (const auto& tag : tags) {
            if (commentDecodedLower.find(tag) != std::string::npos) {
                return true;
            }
        }
        return false;
    }

    std::vector<std::string> CommentListener::getTags() {
        auto tags = Mod::get()->getSettingValue<std::string>("tags");
        auto parts = string::split(tags, ",");

        for (auto& part : parts) {
            part = string::trim(part);
        }
        return parts;
    }

    void CommentListener::onMention(std::string user, std::string msg, std::unordered_map<std::string, std::string> data) {
        if (history::mentionExists(data)) return;
        history::updateHistory(data);

	    log::info("Mention from @{}, '{}'", user, msg);
        m_notifier.notify(
            user + " mentioned you",
            msg
        );
	    // CMUtils::notify(
	    // 	user + " mentioned you",
	    // 	msg
	    // );
    }
}