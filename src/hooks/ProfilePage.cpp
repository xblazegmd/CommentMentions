#include <Geode/Geode.hpp>
#include <Geode/modify/ProfilePage.hpp>

using namespace geode::prelude;

class $modify(CMentionsProfilePage, ProfilePage) {
    bool init(int accountID, bool ownProfile) {
        if (!ProfilePage::init(accountID, ownProfile)) return false;
        auto btmMenu = m_mainLayer->getChildByID("bottom-menu");
        if (!btmMenu) {
            log::error("Could not find bottom-menu (returned null-ptr)");
            return true;
        }

        auto mentionsBt = CCMenuItemSpriteExtra::create(
            CCSprite::createWithSpriteFrameName("GJ_likeBtn_001.png"),
            this,
            menu_selector(CMentionsProfilePage::onMentionsBt)
        );
        btmMenu->addChild(mentionsBt);
        mentionsBt->setID("mentions-bt"_spr);

        btmMenu->updateLayout();
        return true;
    }

    void onMentionsBt(CCObject* sender) {
        FLAlertLayer::create(
            "Mentions",
            "TODO: Mentions stuff ig",
            "Ok"
        )->show();
    }
};