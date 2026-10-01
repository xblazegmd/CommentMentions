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
    this->addChildAtPosition(icon, Anchor::Left, {30, 0});

    // Info menu
    auto infoMenu = CCLayer::create();
    infoMenu->setID("info-menu");
    infoMenu->setAnchorPoint({0, .5f});
    infoMenu->setLayout(
        ColumnLayout::create()
            ->setAxisReverse(true)
            ->setAutoScale(false)
            ->setCrossAxisLineAlignment(AxisAlignment::Start)
            ->setGap(1)
    );

    // Username/Info button menu
    auto usernameMenu = CCLayer::create();
    usernameMenu->setID("title-menu");
    usernameMenu->setAnchorPoint({0, .5f});
    usernameMenu->setLayout(
        RowLayout::create()
            ->setAutoScale(false)
            ->setAxisAlignment(AxisAlignment::Start)
    );
    
    // Username
    auto username = Button::createWithLabel(m_obj.username, "bigFont.fnt", [this](Button*) {
        bool ownProfile = GJAccountManager::get()->m_accountID == m_obj.accountID;
        ProfilePage::create(m_obj.accountID, ownProfile)->show();
    });
    username->setScale(.5f);
    usernameMenu->addChild(username);

    // Info button
    auto infoBtn = Button::createWithSpriteFrameName("GJ_infoIcon_001.png", [this](Button*) {
        FLAlertLayer::create(
            "Mention Info",
            fmt::format("<cy>Username:</c> {}\n", m_obj.username) +
            fmt::format("<co>Level ID:</c> {}", m_obj.levelID),
            "OK"
        )->show();
    });
    infoBtn->setScale(.5f);
    usernameMenu->addChild(infoBtn);

    usernameMenu->updateLayout();
    infoMenu->addChild(usernameMenu);

    // Mention preview
    auto preview = Label::create(this->getCommentPreview(), "chatFont.fnt");
    preview->setScale(.6f);
    preview->setColor({0, 0, 0});
    preview->setOpacity(125);
    infoMenu->addChild(preview);

    infoMenu->updateLayout();
    this->addChildAtPosition(infoMenu, Anchor::Left, {55, 0});

    // "View" button
    auto btnSpr = ButtonSprite::create("View");
    btnSpr->setScale(.7f);
    auto btn = Button::createWithNode(btnSpr, [this](Button*) {
        MentionPopup::create(m_obj)->show();
    });
    this->addChildAtPosition(btn, Anchor::Right, {-50, 0});

    return true;
}

std::string MentionNode::getCommentPreview() {
    auto comment = m_obj.commentt;
    int limit = 30;
    if (comment.length() > limit) {
        comment = comment.substr(0, limit) + "...";
    }
    return fmt::format("\"{}\"", comment);
}
