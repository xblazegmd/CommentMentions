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

bool MentionHistoryPopup::init() {
    if (!Popup::init(440.f, 290.f)) return false;
    m_noElasticity = true;

    // Title
    this->setTitle("Mentions", "bigFont.fnt", .8f);

    // Close button
    setCloseButtonSpr(CCSprite::createWithSpriteFrameName("GJ_backBtn_001.png"));
    static_cast<AnchorLayoutOptions*>(m_closeBtn->getLayoutOptions())->setOffset({10, -10});
    m_buttonMenu->updateLayout();

    // List
    auto listContainer = CCNode::create();
    listContainer->setContentSize(m_listSize);
    listContainer->setAnchorPoint({.5f, .5f});
    listContainer->setID("mentions-list"_spr);

    // Scroll Layer
    auto list = ScrollLayer::create(m_listSize);
    list->m_contentLayer->setLayout(ScrollLayer::createDefaultListLayout(0.f));
    list->setTouchEnabled(true);
    listContainer->addChildAtPosition(list, Anchor::BottomLeft);

    auto mentions = MentionManager::get()->getPreviousMentions();

    // Populate list (iterate backwards cause that's how the list is ordered)
    bool bg = false;
    for (auto it = mentions.rbegin(); it != mentions.rend(); ++it) {
        auto node = MentionNode::create(*it, m_listSize.width);
        node->setBGColor(bg ? m_color1 : m_color2);
        bg = !bg;
        list->m_contentLayer->addChild(node);
    }
    list->m_contentLayer->updateLayout();
    list->moveToTop();

    // Borders
    auto borders = ListBorders::create();
    borders->setContentSize(ccp(m_listSize.width, m_listSize.height + 4));
    listContainer->addChildAtPosition(borders, Anchor::Center);

    m_mainLayer->addChildAtPosition(listContainer, Anchor::Center, {0, -3});

    // Scrollbar
    auto scrollbar = Scrollbar::create(list);
    m_mainLayer->addChildAtPosition(
        scrollbar, Anchor::Center,
        ccp(listContainer->getContentWidth() / 2 + 10, -3)
    );

    return true;
}