#pragma once

#include "Geode/utils/StringMap.hpp"
#include <Geode/Result.hpp>
#include <arc/future/Future.hpp>
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
        Result<> addLevelID(int id);
        Result<> removeLevelID(int id);
    private:
        int m_levelID;
        bool m_running;
        std::vector<int> m_levelIDs;
        async::TaskHolder<void> m_listener;

        arc::Future<> commentEval();
        bool containsMention(const std::string& str);
        void onMention(const std::string& user, const std::string accountID, const std::string& msg);
    };
}