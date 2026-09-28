#pragma once

#include <Geode/ui/Popup.hpp>
#include <Geode/binding/GJComment.hpp>

#include <CommentObject.hpp>

class MentionPopup : public geode::Popup {
public:
    static MentionPopup* create(const CommentObject& obj);
protected:
    bool init(const CommentObject& obj);
    GJComment* objToComment(const CommentObject& obj);
};