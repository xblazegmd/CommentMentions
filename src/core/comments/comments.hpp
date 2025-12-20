#pragma once

#include <core/formatReq/formatReq.hpp>
#include <core/notifier/notifier.hpp>

#include <Geode/utils/Task.hpp>

#include <string>
#include <unordered_map>
#include <vector>

using namespace geode;

namespace comments {
    using ListenerTask = Task<void>;
    using EvalTask = Task<std::vector<std::unordered_map<std::string, formatReq::StrMap>>>;

    class CommentListener {
        public:
            CommentListener(int levelID);

            void start();
            void stop();
        private:
            int m_levelID;
            ListenerTask m_listenerCoro;
            bool m_running = false;
            notifier::Notifier m_notifier;

            ListenerTask startListener();
            EvalTask evalComments();
            bool containsMention(std::string str);
            std::vector<std::string> getTags();
            void onMention(std::string user, std::string msg, std::unordered_map<std::string, std::string> data);
    };
}