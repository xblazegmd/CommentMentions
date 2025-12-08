#include "comments.hpp"

#include <core/utils.hpp>
#include <iostream>
#include <string>

namespace CMentions::comments {
    GDComment::GDComment(int levelID, std::string comment, int authorAccID, int msgID) 
        : levelID_(std::move(levelID)),
          comment_(std::move(comment)),
          authorAccID_(std::move(authorAccID)),
          msgID_(std::move(msgID))
    {}

    void commentListener(int levelID) {
        while (true) {
            // Comment logic
        }
    }

    void lookForMention(int levelID) {
        utils::getDailyLevel();
    }

    void uploadComment() {}
}