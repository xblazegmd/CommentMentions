#include "comments.hpp"

#include <core/utils.hpp>
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

            }
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
                            if (this->containsMention(comment)) {
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
        auto strSplit = string::split(str, ":");
        auto commentStr = string::split(strSplit[0], "~");
        auto authorStr = string::split(strSplit[1], "~");
        return true;
    }
}