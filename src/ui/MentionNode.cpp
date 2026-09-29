#include "MentionNode.hpp"

#include <MentionManager.hpp>
#include <ui/MentionPopup.hpp>

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

void MentionNode::setBGColor(bool color1) {
    m_bg->setColor(color1 ? ccc3(191, 114, 62) : ccc3(161, 88, 44));
}

bool MentionNode::init(const CommentObject& obj, float width) {
    if (!CCNode::init()) return false;
    this->setContentSize({width, 40});
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
    icon->setScale(.8f);
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
            ->setGap(1)
    );

    // Username
    auto username = Button::createWithLabel(m_obj.username, "bigFont.fnt", [this](Button*) {
        bool ownProfile = GJAccountManager::get()->m_accountID == m_obj.accountID;
        ProfilePage::create(m_obj.accountID, ownProfile)->show();
    });
    username->setScale(.5f);
    labels->addChild(username);

    // Mention preview
    auto preview = Label::create(fmt::format("Level: {}", m_obj.levelID), "chatFont.fnt");
    preview->setScale(.5f);
    preview->setColor({0, 0, 0});
    preview->setOpacity(125);
    labels->addChild(preview);

    labels->updateLayout();
    this->addChildAtPosition(labels, Anchor::Left, {65, 0});

    // "View" button
    auto btnSpr = ButtonSprite::create("View");
    btnSpr->setScale(.7f);
    auto btn = Button::createWithNode(btnSpr, [this](Button*) {
        MentionPopup::create(m_obj)->show();
        // FLAlertLayer::create(
        //     fmt::format("@{}", m_obj.username).c_str(),
        //     m_obj.commentt.c_str(),
        //     "OK"
        // )->show();
    });
    this->addChildAtPosition(btn, Anchor::Right, {-50, 0});

    return true;
}
