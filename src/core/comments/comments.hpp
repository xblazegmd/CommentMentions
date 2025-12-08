#pragma once
#include <iostream>
#include <string>

namespace CMentions::comments {
    class GDComment {
        public:
            GDComment(int levelID, std::string comment, int authorAccID, int msgID);

            GDComment formatResponse(std::string response);

            bool isInComment(std::string tag);
        private:
            int levelID_;
            std::string comment_;
            int authorAccID_;
            int msgID_;
    };

    void commentListener(int levelID);

    void lookForMention(int levelID);

    void uploadComment();
}