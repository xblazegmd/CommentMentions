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
    CommentManager::CommentManager() = default;

    void CommentManager::startAll() {
        m_listener.spawn(
            "CommentManager",
            commentEval(),
            [] {}
        );
    }

    void CommentManager::stopAll() {
        m_listener.cancel();
    }

    void CommentManager::addLevelID(int id) {
        m_levelIDs.push_back(id);
    }

    void CommentManager::removeLevelID(int id) {
        // TODO: Safely remove ID without breaking everything
    }

    arc::Future<> CommentManager::commentEval() {
        while (true) {
            for (const int& levelID : m_levelIDs) {
                std::vector<CMUtils::CommentObject> foundComments;

                auto req = web::WebRequest()
                    .userAgent("")
                    .timeout(std::chrono::seconds(10))
                    .bodyString("levelID=" + utils::numToString(levelID) + "&page=0&secret=" + CMUtils::SECRET);

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
                    log::error("Failed to fetch comments for ID '{}'", levelID);
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