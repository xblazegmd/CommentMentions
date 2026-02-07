#pragma once

#include <arc/prelude.hpp>
#include <Geode/Result.hpp>
// #include <core/notifier/notifier.hpp>

#include <Geode/utils/async.hpp>

#include <string>
#include <vector>

using namespace geode;

namespace comments {
    class CommentManager {
    public:
        CommentManager();

        void startAll();
        void stopAll();
        void addTargetID(int id);
        void removeTargetID(int id);
    private:
        bool m_running;
        arc::Mutex<std::vector<int>> m_targets;
        async::TaskHolder<void> m_listener;

        arc::Future<> commentEval();
        bool containsMention(const std::string& str);
        void onMention(const std::string& user, const std::string accountID, const std::string& msg);
    };
}