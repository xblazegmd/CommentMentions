#pragma once

#include "Geode/utils/StringMap.hpp"
#include <arc/future/Future.hpp>
// #include <core/notifier/notifier.hpp>

#include <Geode/utils/async.hpp>

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

using namespace geode;

namespace comments {
    // using ListenerTask = Task<void>;
    // using EvalTask = Task<std::vector<std::unordered_map<std::string, formatReq::StrMap>>>;

    class CommentManager {
    public:
        CommentManager(int levelID);

        void start();
        void stop();
    private:
        int m_levelID;
        async::TaskHolder<void> m_listener;

        arc::Future<> commentEval();
        bool containsMention(const std::string& str);
        void onMention(const std::string& user, const std::string accountID, const std::string& msg);
        // public:
        //     CommentListener(int levelID);

        //     void start();
        //     void stop();
        // private:
        //     int m_levelID;
        //     ListenerTask m_listenerCoro;
        //     bool m_running = false;
        //     notifier::Notifier m_notifier;

        //     ListenerTask startListener();
        //     EvalTask evalComments();
        //     bool containsMention(std::string str);
        //     std::vector<std::string> getTags();
        //     void onMention(std::string user, std::string msg, std::unordered_map<std::string, std::string> data);
    };
}