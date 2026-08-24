#pragma once

#include <Geode/ui/Popup.hpp>
#include <Geode/ui/ScrollLayer.hpp>
#include <MentionManager.hpp>

#include <ui/MentionNode.hpp>

class MentionHistoryPopup : public geode::Popup {
public:
    static MentionHistoryPopup* create() {
        auto ret = new MentionHistoryPopup();
        if (ret->init()) {
            ret->autorelease();
            return ret;
        }
        delete ret;
        return nullptr;
    }
private:
    const cocos2d::CCSize m_listSize = {340, 196};
    const cocos2d::ccColor3B m_color1 = {191, 114, 62};
    const cocos2d::ccColor3B m_color2 = {161, 88, 44};

    bool init();
};