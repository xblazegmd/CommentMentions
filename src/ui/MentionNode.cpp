#include "MentionNode.hpp"

#include <Geode/Geode.hpp>
#include <Geode/Enums.hpp>
#include <Geode/ui/Layout.hpp>
#include <Geode/utils/general.hpp>
#include <Geode/binding/SimplePlayer.hpp>

#include <MentionManager.hpp>

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
    this->setContentSize({width, 45});

    // Background
    m_bg = CCLayerColor::create({0, 0, 0, 255});
    m_bg->setContentSize(this->getContentSize());
    m_bg->ignoreAnchorPointForPosition(false);
    m_bg->setAnchorPoint({.5f, .5f});
    this->addChildAtPosition(m_bg, Anchor::Center);

    // Main layer
    auto mainLayer = CCLayer::create();
    mainLayer->setLayout(
        RowLayout::create()
            ->setAutoScale(false)
            ->setGap(25)
    );

    // Player Icon
    // All of this mess is temporary while I make CommentObject store this data more nicely
    int iconID = utils::numFromString<int>(obj.author.at("icon")).unwrapOr(1);
    auto type = utils::numFromString<int>(obj.author.at("iconType")).unwrapOr(1);
    IconType iconType = static_cast<IconType>(type);
    int colorA = utils::numFromString<int>(obj.author.at("playerColor")).unwrapOr(1);
    int colorB = utils::numFromString<int>(obj.author.at("playerColor2")).unwrapOr(1);
    int glow = utils::numFromString<int>(obj.author.at("glow")).unwrapOr(1);

    auto icon = SimplePlayer::create(iconID);
    icon->updatePlayerFrame(iconID, iconType);
    icon->setColors(GameManager::get()->colorForIdx(colorA), GameManager::get()->colorForIdx(colorB));
    icon->setGlowOutline(GameManager::get()->colorForIdx(colorB));

    if (!glow) icon->disableGlowOutline();

    mainLayer->addChild(icon);

    // Username label
    auto username = CCLabelBMFont::create(obj.author.at("userName").c_str(), "bigFont.fnt");
    username->setScale(.8f);
    mainLayer->addChild(username);

    mainLayer->updateLayout();
    this->addChildAtPosition(mainLayer, Anchor::Center);
    return true;
}