#include "MentionNode.hpp"

#include <MentionManager.hpp>

#include <Geode/Geode.hpp>
#include <Geode/Enums.hpp>

#include <Geode/ui/Layout.hpp>
#include <Geode/ui/Label.hpp>
#include <Geode/ui/Button.hpp>

#include <Geode/utils/general.hpp>
#include <Geode/binding/SimplePlayer.hpp>
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/ProfilePage.hpp>
#include <Geode/binding/LevelBrowserLayer.hpp>

using namespace geode::prelude;

MentionNode* MentionNode::create(const CommentObject &obj, float width) {
    auto ret = new MentionNode();
    if (ret->init(obj, width)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

void MentionNode::setBGColor(ccColor3B color) {
    m_bg->setColor(color);
}

bool MentionNode::init(const CommentObject& obj, float width) {
    if (!CCNode::init()) return false;
    this->setContentSize({width, 50});
    m_obj = obj;

    // Background
    m_bg = CCLayerColor::create({0, 0, 0, 255});
    m_bg->setContentSize(this->getContentSize());
    m_bg->ignoreAnchorPointForPosition(false);
    m_bg->setAnchorPoint({.5f, .5f});
    this->addChildAtPosition(m_bg, Anchor::Center);

    // Player Icon
    auto gameManager = GameManager::get();

    auto icon = SimplePlayer::create(m_obj.iconID);
    icon->updatePlayerFrame(m_obj.iconID, static_cast<IconType>(m_obj.iconType));
    icon->setColors(gameManager->colorForIdx(m_obj.color1), gameManager->colorForIdx(m_obj.color2));
    icon->setGlowOutline(gameManager->colorForIdx(m_obj.color3));
    if (!m_obj.glow) icon->disableGlowOutline();

    icon->setAnchorPoint({0, .5f});
    this->addChildAtPosition(icon, Anchor::Left, {40, 0});

    // Labels
    auto labels = CCLayer::create();
    labels->setID("labels");
    labels->setAnchorPoint({0, .5f});
    labels->setLayout(
        ColumnLayout::create()
            ->setAxisReverse(true)
            ->setAutoScale(false)
            ->setCrossAxisLineAlignment(AxisAlignment::Start)
            ->setGap(0)
    );

    // Username
    auto username = Button::createWithLabel(m_obj.username, "bigFont.fnt", [this](Button*) {
        bool ownProfile = GJAccountManager::get()->m_accountID == m_obj.accountID;
        ProfilePage::create(m_obj.accountID, ownProfile)->show();
    });
    username->setScale(.6f);
    labels->addChild(username);

    // Level ID
    auto levelID = Button::createWithLabel(fmt::format("Level: {}", m_obj.levelID), "goldFont.fnt", [this](Button*) {
        auto searchObject = GJSearchObject::create(SearchType::Type19, fmt::format("{}&gameVersion=22", m_obj.levelID));
        auto scene = LevelBrowserLayer::scene(searchObject);
        CCDirector::get()->replaceScene(CCTransitionFade::create(.5f, scene));
    });
    levelID->setScale(.5f);
    labels->addChild(levelID);

    labels->updateLayout();
    this->addChildAtPosition(labels, Anchor::Left, {70, 0});

    // "View" button
    auto btnSpr = ButtonSprite::create("View");
    btnSpr->setScale(.8f);
    auto btn = Button::createWithNode(btnSpr, [this](Button*) {
        FLAlertLayer::create(
            fmt::format("@{}", m_obj.username).c_str(),
            m_obj.commentt.c_str(),
            "OK"
        )->show();
    });
    this->addChildAtPosition(btn, Anchor::Right, {-50, 0});

    return true;
}