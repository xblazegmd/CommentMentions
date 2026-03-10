// TODO: Uncomment and finish when working on v1.1
// #include <Geode/Geode.hpp>
// #include <Geode/modify/ProfilePage.hpp>

// using namespace geode::prelude;

// class $modify(PPHook, ProfilePage) {
//     void loadPageFromUserInfo(GJUserScore* score) {
//         ProfilePage::loadPageFromUserInfo(score);
//         if (!m_ownProfile) return;

//         auto btmMenu = m_mainLayer->getChildByID("bottom-menu");
//         if (!btmMenu) {
//             log::error("Could not find bottom-menu");
//             return;
//         }

//         auto btn = CCMenuItemSpriteExtra::create(
//             CCSprite::createWithSpriteFrameName("GJ_likeBtn_001.png"),
//             this,
//             menu_selector(PPHook::onMentions)
//         );
//         btn->setID("mentions-button");

//         btmMenu->addChild(btn);
//         btmMenu->updateLayout();
//     }

//     void onMentions(CCObject*) {
//         FLAlertLayer::create(
//             "TODO",
//             "todo",
//             "todo?"
//         )->show();
//     }
// };