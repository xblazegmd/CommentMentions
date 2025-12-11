#include "comments.hpp"

#include <core/utils.hpp>
#include <core/formatReq/formatReq.hpp>
#include <Geode/loader/Mod.hpp>
#include <Geode/utils/base64.hpp>
#include <Geode/utils/coro.hpp>
#include <Geode/utils/string.hpp>
#include <Geode/utils/Task.hpp>
#include <Geode/utils/web.hpp>
#include <chrono>
#include <functional>
#include <string>

using namespace geode;
using namespace geode::utils;

namespace comments {
    CommentListener::CommentListener(int levelID, std::function<void(std::string, std::string)> onMentionCallback) :
        m_levelID(levelID),
        m_onMentionCallback(onMentionCallback)
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

                    m_onMentionCallback(mention["authorStr"]["username"], commentDecoded);
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
}