#include <Geode/Geode.hpp>
#include <Geode/modify/ProfilePage.hpp>

using namespace geode::prelude;

class $modify(CMProfilePage, ProfilePage) {
    void getUserInfoFinished(GJUserScore* score) {
        ProfilePage::getUserInfoFinished(score);

        if (!m_ownProfile) {
            log::info("Not own profile");
            return;
        }

        auto btmMenu = m_mainLayer->getChildByID("bottom-menu");
        if (!btmMenu) {
            log::error("Could not find bottom-menu (returned nullptr)");
            return;
        }

        auto mentionsBt = CCMenuItemSpriteExtra::create(
            CCSprite::create("CMProfileMentionsBt.png"_spr),
            this,
            menu_selector(CMProfilePage::onMentionsBt)
        );

        btmMenu->addChild(mentionsBt);
        mentionsBt->setID("mentions-bt"_spr);

        btmMenu->updateLayout();
    }

    void onMentionsBt(CCObject* sender) {
        FLAlertLayer::create(
            "Coming Soon!",
            "Your mention history will be available here",
            "OK"
        )->show();
    }
};