#include "CMMentionsPage.hpp"
#include "Geode/cocos/platform/CCPlatformMacros.h"

#include <Geode/Geode.hpp>

using namespace geode::prelude;

namespace layers {
    CMMentionsPage* CMMentionsPage::create() {
        auto ret = new CMMentionsPage();
        if (ret && ret->init()) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }

    bool CMMentionsPage::init() {
        if (!CCLayer::init()) return false;

        auto bg = CCScale9Sprite::create("square_02_001.png");
        bg->setContentSize({365.f, 40.f});
        bg->setOpacity(100);
        this->addChild(bg);
        
        this->scheduleUpdate();
        return true;
    }
}