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
    this->setContentSize({width, 60});

    // Background
    m_bg = CCLayerColor::create({0, 0, 0, 255});
    m_bg->setContentSize(this->getContentSize());
    m_bg->ignoreAnchorPointForPosition(false);
    m_bg->setAnchorPoint({.5f, .5f});
    this->addChildAtPosition(m_bg, Anchor::Center);

    // Player Icon
    auto gameManager = GameManager::get();

    auto icon = SimplePlayer::create(obj.iconID);
    icon->updatePlayerFrame(obj.iconID, static_cast<IconType>(obj.iconType));
    icon->setColors(gameManager->colorForIdx(obj.color1), gameManager->colorForIdx(obj.color2));
    icon->setGlowOutline(gameManager->colorForIdx(obj.color3));
    if (!obj.glow) icon->disableGlowOutline();

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
    auto username = Label::create(obj.username, "bigFont.fnt"); // trying out geode::Label for this one
    username->setScale(.6f);
    labels->addChild(username);

    // Level ID
    auto levelID = Label::create(fmt::format("Level: {}", obj.levelID), "goldFont.fnt");
    levelID->setScale(.5f);
    labels->addChild(levelID);

    labels->updateLayout();
    this->addChildAtPosition(labels, Anchor::Left, {70, 0});

    // "View" button
    auto btnSpr = ButtonSprite::create("View");
    btnSpr->setScale(.8f);
    auto btn = Button::createWithNode(btnSpr, [obj](Button*) {
        FLAlertLayer::create(
            fmt::format("@{}", obj.username).c_str(),
            obj.commentt.c_str(),
            "OK"
        )->show();
    });
    this->addChildAtPosition(btn, Anchor::Right, {-50, 0});

    return true;
}