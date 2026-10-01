#pragma once

#include <Geode/Geode.hpp>
#include <CommentObject.hpp>

class MentionNode : public cocos2d::CCNode {
public:
    static MentionNode* create(const CommentObject& obj, float width);

    void setBGColor(bool color1);
private:
    CommentObject m_obj;
    cocos2d::CCLayerColor* m_bg;

    bool init(const CommentObject& obj, float width);
    std::string getCommentPreview();
};
