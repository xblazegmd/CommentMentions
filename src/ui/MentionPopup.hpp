#pragma once

#include <Geode/ui/Popup.hpp>
#include <CommentObject.hpp>
#include <regex>

class MentionPopup : public geode::Popup {
public:
    static MentionPopup* create(const CommentObject& obj);
protected:
    const cocos2d::CCSize m_commentAreaSize = {287, 115};
    const std::regex m_mentionRegex = std::regex("@\\w+");

    CommentObject m_obj;

    bool init(const CommentObject& obj);
    std::string getCommentWithHighlight();
};
