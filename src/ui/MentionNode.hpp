#pragma once

#include <Geode/Geode.hpp>
#include <MentionManager.hpp>

class MentionNode : public cocos2d::CCNode {
public:
    static MentionNode* create(const CommentObject& obj, float width);

    void setBGColor(cocos2d::ccColor3B color);
private:
    CommentObject m_obj;
    cocos2d::CCLayerColor* m_bg;

    bool init(const CommentObject& obj, float width);
};