#pragma once

#include <functional>
#include <string>

namespace comments {
    class CommentListener {
        public:
            CommentListener(int levelID, std::function<void(std::string, std::string)> onMentionCallback);

            void start();

            void stop();
        private:
            int m_levelID;
            std::function<void(std::string, std::string)> m_onMentionCallback;

            void checkForMentions();
    };
}