#pragma once

#include <Geode/utils/Task.hpp>
#include <functional>
#include <string>
#include <vector>

using namespace geode;

namespace comments {
    using ListenerTask = Task<void>;

    class CommentListener {
        public:
            CommentListener(int levelID, std::function<void(std::string, std::string)> onMentionCallback);

            void start();

            void stop();
        private:
            int m_levelID;
            std::function<void(std::string, std::string)> m_onMentionCallback;

            ListenerTask startListener();

            Task<std::vector<std::string>> evalComments();

            bool containsMention(std::string str);
    };
}