#include "comments.hpp"
#include "Geode/utils/StringMap.hpp"
#include "Geode/utils/general.hpp"

#include <algorithm>
#include <arc/future/Future.hpp>
#include <arc/sync/Mutex.hpp>
#include <arc/time/Sleep.hpp>
#include <asp/time/Duration.hpp>
#include <core/utils.hpp>

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
    CommentManager::~CommentManager() {
        stopAll();
        deinitHistory();
    }

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
                std::vector<CommentObject> foundComments;

                auto req = web::WebRequest()
                    .userAgent("")
                    .timeout(std::chrono::seconds(10))
                    .bodyString("levelID=" + utils::numToString(target) + "&page=0&secret=" + SECRET);

                auto res = co_await req.post(BOOMLINGS + "getGJComments21.php");
                if (res.ok() && stringIsOk(res.string())) {
                    auto comments = string::split(res.string().unwrap(), "|");
                    for (const auto& comment : comments) {
                        auto obj = formatCommentObj(comment);
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
                        auto msgID = comment.comment.find("messageID");

                        onMention(
                            username != comment.author.end() ? username->second : "User",
                            accountID != comment.author.end() ? accountID->second : "-1",
                            msg != comment.comment.end() ? msg->second : "N/A",
                            msgID != comment.comment.end() ? msgID->second : "-1"
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

    void CommentManager::onMention(
        const std::string& user,
        const std::string accountID,
        const std::string& msg,
        const std::string& msgID
    ) {
        // if (isOnHistory(msgID)) return;
        // addToHistory(user, accountID, msg, msgID);
        // m_notifier.notify(user + " mentioned you", msg);
        log::info("Mention from @{}: '{}'", user, msg);
    }

    void CommentManager::initHistory() {
        m_mentionHistory = Mod::get()->getSavedValue<History>("history");
        handleHistoryMaxSize();
        m_saveHistory.spawn(
            "History Task",
            saveHistoryTask(),
            [] {}
        );
    }


    void CommentManager::deinitHistory() {
        saveHistory();
        m_saveHistory.cancel();
    }

    void CommentManager::addToHistory(const std::string& user, const std::string accountID, const std::string& msg, const std::string& msgID) {
        m_mentionHistory.push_back({
            { "user", user },
            { "accountID", accountID },
            { "msg", msg },
            { "msgID", msgID }
        });
        handleHistoryMaxSize();
    }

    bool CommentManager::isOnHistory(const std::string& msg) {
        for (const auto& mention : m_mentionHistory) {
            auto msgIt = mention.find("msgID");
            if (msgIt == mention.end()) return false;
            if (msgIt->second == msg) return true;
        }
        return false;
    }

    void CommentManager::saveHistory() {
        handleHistoryMaxSize();
        Mod::get()->setSavedValue<History>("history", m_mentionHistory);
    }

    arc::Future<> CommentManager::saveHistoryTask() {
        while (true) {
            co_await arc::sleep(asp::Duration::fromSecs(100));
            saveHistory();
        }
    }

    void CommentManager::handleHistoryMaxSize() {
        auto maxsize = Mod::get()->getSettingValue<int64_t>("history-maxsize");
        if (m_mentionHistory.size() > maxsize) {
            m_mentionHistory.erase(m_mentionHistory.begin());
        }
    }
}