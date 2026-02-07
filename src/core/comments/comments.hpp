#pragma once

#include <arc/prelude.hpp>
#include <core/utils.hpp>
#include <Geode/Result.hpp>
#include <core/notifier/notifier.hpp>

#include <Geode/utils/async.hpp>

#include <string>
#include <vector>

using namespace geode;

namespace CommentMentions {
    class CommentManager {
    public:
        CommentManager() = default;
        ~CommentManager();

        void startAll();
        void stopAll();
        void addTargetID(int id);
        void removeTargetID(int id);
    private:
        arc::Mutex<std::vector<int>> m_targets;
        async::TaskHolder<void> m_listener;
        Notifier m_notifier;

        using History = std::vector<utils::StringMap<std::string>>;
        History m_mentionHistory;
        async::TaskHolder<void> m_saveHistory;

        arc::Future<> commentEval();
        bool containsMention(const std::string& str);
        void onMention(
            const std::string& user,
            const std::string accountID,
            const std::string& msg,
            const std::string& msgID
        );

        void initHistory();
        void deinitHistory();
        void addToHistory(
            const std::string& user,
            const std::string accountID,
            const std::string& msg,
            const std::string& msgID
        );
        bool isOnHistory(const std::string& msgID);
        void saveHistory();
        arc::Future<> saveHistoryTask();
        void handleHistoryMaxSize();
    };
}