#include "comments.hpp"

#include <algorithm>
#include <arc/future/Future.hpp>
#include <arc/sync/Mutex.hpp>
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

namespace CommentMentions {
    CommentManager::CommentManager() = default;

    void CommentManager::startAll() {
        m_listener.spawn(
            "CommentManager",
            commentEval(),
            [] {}
        );
        m_running = true;
    }

    void CommentManager::stopAll() {
        m_listener.cancel();
        m_running = false;
    }

    void CommentManager::addTargetID(int id) {
        async::spawn([this, id]() -> arc::Future<> {
            auto lock = co_await m_targets.lock();
            lock->push_back(id);
        });
    }

    void CommentManager::removeTargetID(int id) {
        async::spawn([this, id]() -> arc::Future<> {
            auto lock = co_await m_targets.lock();
            lock->erase(
                std::remove(lock->begin(), lock->end(), id),
                lock->end()
            );
        });
    }

    arc::Future<> CommentManager::commentEval() {
        while (true) {
            auto lock = co_await m_targets.lock();
            for (const int& target : *lock) {
                std::vector<CommentMentions::CommentObject> foundComments;

                auto req = web::WebRequest()
                    .userAgent("")
                    .timeout(std::chrono::seconds(10))
                    .bodyString("levelID=" + utils::numToString(target) + "&page=0&secret=" + CommentMentions::SECRET);

                auto res = co_await req.post(CommentMentions::BOOMLINGS + "getGJComments21.php");
                if (res.ok() && CommentMentions::stringIsOk(res.string())) {
                    auto comments = string::split(res.string().unwrap(), "|");
                    for (const auto& comment : comments) {
                        auto obj = CommentMentions::formatCommentObj(comment);
                        if (containsMention(obj.comment["comment"])) {
                            foundComments.push_back(obj);
                        }
                    }
                } else {
                    log::error("Failed to fetch comments for ID '{}'", target);
                    log::info("Status code: {}", res.code());
                    log::info("Response: '{}'", res.string().unwrapOr("N/A"));
                }

                if (!foundComments.empty()) {
                    for (const auto& comment : foundComments) {
                        auto username = comment.author.find("userName");
                        auto accountID = comment.author.find("accountID");
                        auto msg = comment.comment.find("comment");

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
    }

    bool CommentManager::containsMention(const std::string& str) {
        std::vector<std::string> tags{ "xblazegmd", "xblaze", "blaze" }; // Hardcoded for testing
        auto lower = string::toLower(str);
        for (const auto& tag : tags) {
            if (string::contains(lower, tag)) return true;
        }
        return false;
    }

    void CommentManager::onMention(const std::string& user, const std::string accountID, const std::string& msg) {
        log::info("Mention from @{}: '{}'", user, msg);
    }
}