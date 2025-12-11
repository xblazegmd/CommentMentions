#pragma once

#include <core/formatReq/formatReq.hpp>
#include <Geode/utils/Task.hpp>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

using namespace geode;

namespace comments {
    using ListenerTask = Task<void>;
    using EvalTask = Task<std::vector<std::unordered_map<std::string, formatReq::StrMap>>>;

    class CommentListener {
        public:
            CommentListener(int levelID, std::function<void(std::string, std::string, std::string)> onMentionCallback);

            void start();
            void stop();
        private:
            int m_levelID;
            std::function<void(std::string, std::string, std::string)> m_onMentionCallback;
            ListenerTask m_listenerCoro;
            bool m_running = false;

            ListenerTask startListener();
            EvalTask evalComments();
            bool containsMention(std::string str);
            std::vector<std::string> getTags();
    };
}