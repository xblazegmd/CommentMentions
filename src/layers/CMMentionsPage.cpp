#include "CMMentionsPage.hpp"

#include <Geode/Geode.hpp>

using namespace geode::prelude;

namespace layers {
    bool CMMentionsPage::init() {
        if (!CCLayer::init()) return false;
        return true;
    }

    CMMentionsPage* CMMentionsPage::create() {
        auto cmmp = new CMMentionsPage();
        if (cmmp && cmmp->init()) {
            cmmp->autorelease();
            return cmmp;
        }
        CC_SAFE_DELETE(cmmp);
        return nullptr;
    }
}