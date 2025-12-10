#include "comments.hpp"

#include <core/utils.hpp>
#include <core/formatReq/formatReq.hpp>
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
        coro::spawn << startListener();
    }

    ListenerTask CommentListener::startListener() {
        while (true) {
            auto mentions = co_await evalComments();

            if (!mentions.empty()) {
                for (const auto& mention : mentions) {
                    /* 
                        Why not, idk, make the function return the comment object maybe? idk, it's a pain to change how that function works anyways
                        so I'll stick with this for now
                    */
                    auto commentObj = formatReq::formatCommentObj(mention);

                    auto commentDecodedRes = base64::decode(commentObj["commentStr"]["comment"], base64::Base64Variant::Url);
                    if (commentDecodedRes.isErr()) {
                        log::error("Could not decode comment '{}': {}", commentObj["commentStr"]["comment"], commentDecodedRes.unwrapErr());
                        continue;
                    }

                    auto bytes = commentDecodedRes.unwrap();
                    std::string commentDecoded(bytes.begin(), bytes.end());

                    m_onMentionCallback(commentObj["authorStr"]["username"], commentDecoded);
                }
            }

            co_await coro::sleep(10);
        }
    }

    Task<std::vector<std::string>> CommentListener::evalComments() {
        return Task<std::vector<std::string>>::runWithCallback([this](auto finish, auto prog, auto isCancelled) {
            std::string params = "levelID=" + std::to_string(this->m_levelID) + "&page=0&secret=" + CMutils::SECRET;

            auto req = web::WebRequest();
            req.userAgent("");
            req.timeout(std::chrono::seconds(10));
            req.bodyString(params);

            req.post(CMutils::BOOMLINGS + "getGJComments21.php")
                .listen([this, finish](web::WebResponse *res) {
                    if (res && res->ok() && res->string().isOk()) {
                        auto comments = string::split(res->string().unwrap(), "|");
                        std::vector<std::string> foundComments;

                        for (std::string comment : comments) {
                            auto commentObj = formatReq::formatCommentObj(comment);
                            if (this->containsMention(commentObj["commentStr"]["comment"])) {
                                foundComments.push_back(comment);
                            }
                        }

                        finish(foundComments);
                    } else {
                        log::error("Could not fetch comments ({}, response: {})", res->code(), res->string().unwrapOr("..."));
                        std::vector<std::string> emptyLol;
                        finish(emptyLol);
                    }
                });
        }, "evalComments");
    }

    bool CommentListener::containsMention(std::string str) {
        std::vector<std::string> tags = { "xblazegmd", "xblaze", "blaze" };

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
}