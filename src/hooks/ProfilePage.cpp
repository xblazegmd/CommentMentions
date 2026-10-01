#include <Geode/Geode.hpp>
#include <Geode/ui/BasedButtonSprite.hpp>
#include <Geode/ui/Label.hpp>
#include <Geode/modify/ProfilePage.hpp>

#include <ui/MentionHistoryPopup.hpp>

using namespace geode::prelude;

class $modify(PPHook, ProfilePage) {
    struct Fields {
        bool m_loaded = false;
    };

    void loadPageFromUserInfo(GJUserScore* score) {
        ProfilePage::loadPageFromUserInfo(score);
        if (!m_ownProfile) return;

        if (auto leftMenu = m_mainLayer->getChildByID("left-menu")) {
            if (m_fields->m_loaded) return;
            m_fields->m_loaded = true;

            // Using text cuz I'm lazy :P
            auto spr = CircleButtonSprite::create(CCSprite::create("mentionIcon.png"_spr), CircleBaseColor::Cyan);
            spr->setScale(.7f);

            auto btn = CCMenuItemSpriteExtra::create(spr, this, menu_selector(PPHook::onMentions));
            btn->setID("mentions-btn"_spr);
            leftMenu->addChild(btn);
            leftMenu->updateLayout();
        }
    }

    void onMentions(CCObject*) {
        MentionHistoryPopup::create(this)->show();
    }
};
