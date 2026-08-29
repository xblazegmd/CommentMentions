#include "MentionHistoryPopup.hpp"
#include "Geode/ui/Scrollbar.hpp"

#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/General.hpp>
#include <Geode/ui/ScrollLayer.hpp>
#include <Geode/ui/Layout.hpp>

#include <ui/MentionNode.hpp>

#include <MentionManager.hpp>

using namespace geode::prelude;

bool MentionHistoryPopup::init(ProfilePage* profilePage) {
    if (!Popup::init(440.f, 290.f)) return false;
    m_noElasticity = true;
    m_profilePage = profilePage;
    m_profilePage->setVisible(false);

    // Title
    this->setTitle("Mentions", "bigFont.fnt", .8f);

    // Close button
    this->setCloseButtonSpr(CCSprite::createWithSpriteFrameName("GJ_backBtn_001.png"));
    static_cast<AnchorLayoutOptions*>(m_closeBtn->getLayoutOptions())->setOffset({10, -10});
    m_buttonMenu->updateLayout();

    // Refresh button
    auto refresh = CCMenuItemSpriteExtra::create(
        CCSprite::createWithSpriteFrameName("GJ_updateBtn_001.png"),
        this,
        menu_selector(MentionHistoryPopup::onRefresh)
    );
    m_buttonMenu->addChildAtPosition(refresh, Anchor::BottomRight, {-10, 10});

    // List
    auto listContainer = CCLayerColor::create(to4B(m_color1));
    listContainer->setContentSize(m_listSize);
    listContainer->setAnchorPoint({.5f, .5f});
    listContainer->setID("mentions-list"_spr);

    // Scroll Layer
    m_list = ScrollLayer::create(m_listSize);
    m_list->m_contentLayer->setLayout(ScrollLayer::createDefaultListLayout(0.f));
    m_list->setTouchEnabled(true);
    listContainer->addChildAtPosition(m_list, Anchor::BottomLeft);

    // Borders
    auto borders = ListBorders::create();
    borders->setContentSize(ccp(m_listSize.width, m_listSize.height + 4));
    listContainer->addChildAtPosition(borders, Anchor::Center);

    m_mainLayer->addChildAtPosition(listContainer, Anchor::Center, {0, -3});

    // Scrollbar
    auto scrollbar = Scrollbar::create(m_list);
    m_mainLayer->addChildAtPosition(
        scrollbar, Anchor::Center,
        ccp(listContainer->getContentWidth() / 2 + 10, -3)
    );

    this->populateList();
    return true;
}

void MentionHistoryPopup::onClose(CCObject* sender) {
    Popup::onClose(sender);
    m_profilePage->setVisible(true);
}

void MentionHistoryPopup::populateList() {
    m_list->m_contentLayer->removeAllChildren();

    auto mentions = MentionManager::get()->getPreviousMentions();

    // Iterate backwards cuz that's how the list is ordered
    bool bg = false;
    for (auto it = mentions.rbegin(); it != mentions.rend(); ++it) {
        auto node = MentionNode::create(*it, m_listSize.width);
        node->setBGColor(bg ? m_color1 : m_color2);
        bg = !bg;
        m_list->m_contentLayer->addChild(node);
    }
    m_list->m_contentLayer->updateLayout();
    m_list->moveToTop();
}

void MentionHistoryPopup::onRefresh(CCObject*) {
    if (!m_list) return;
    this->populateList();
}