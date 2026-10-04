#include "ReplyPopup.hpp"

#include <Geode/Enums.hpp>
#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/TextInput.hpp>
#include <Geode/ui/Label.hpp>
#include <Geode/ui/Layout.hpp>
#include <Geode/ui/LoadingSpinner.hpp>

#include <Geode/binding/CommentUploadDelegate.hpp>

#include <string>

using namespace geode::prelude;

// ReplyPopup
ReplyPopup* ReplyPopup::create(int levelID, const std::string& username) {
    auto ret = new ReplyPopup();
    if (ret->init(levelID, username)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool ReplyPopup::init(int levelID, const std::string& username) {
    if (!Popup::init(400, 135)) return false;
    m_levelID = levelID;
    m_username = username;
    m_noElasticity = true;

    this->setTitle("Reply", "bigFont.fnt");

    // Text Input
    m_textInput = TextInput::create(360, "Comment", "chatFont.fnt");
    m_textInput->setString(fmt::format("@{} ", m_username));
    m_textInput->setMaxCharCount(100);
    m_textInput->setCommonFilter(CommonFilter::Any);
    m_textInput->getBGSprite()->setContentHeight(90);

    m_textInput->setCallback([this](const std::string& str) {
        int remaining = this->getRemainingChars();
        m_charLabel->setText(utils::numToString(remaining));
        if (remaining <= 10) {
            m_charLabel->setColor({255, 0, 0});
        } else {
            m_charLabel->setColor({0, 0, 0});
        }
    });

    m_mainLayer->addChildAtPosition(m_textInput, Anchor::Center, {0, 5});

    // Remaining characters label
    m_charLabel = Label::create(utils::numToString(this->getRemainingChars()), "chatFont.fnt");
    m_charLabel->setColor({0, 0, 0});
    m_charLabel->setOpacity(125);
    m_charLabel->setAnchorPoint({1, .5f});
    m_mainLayer->addChildAtPosition(m_charLabel, Anchor::TopRight, {-20, -20});

    // Buttons
    auto btnMenu = CCMenu::create();
    btnMenu->setID("buttons"_spr);
    btnMenu->setContentSize(m_textInput->getContentSize());
    btnMenu->setAnchorPoint({.5f, .5f});
    btnMenu->setLayout(
        RowLayout::create()
            ->setAutoScale(false)
    );

    // Cancel
    auto cancelCb = [this](Button*) {
        this->onClose(nullptr);
    };

    // Reply
    auto replyCb = [this](Button*) {
        // This will also upload the comment as well
        ReplyUploadPopup::create(m_levelID, m_textInput->getString(), this)->show();
        m_textInput->defocus();
    };

    for (auto& item : std::array<std::tuple<const char*, Button::ButtonCallback>, 2>{{
        {"Cancel", std::move(cancelCb)},
        {"Reply", std::move(replyCb)},
    }}) {
        auto btnSpr = ButtonSprite::create(std::get<0>(item));
        btnSpr->setScale(.9f);
        auto btn = Button::createWithNode(btnSpr, std::move(std::get<1>(item)));
        btnMenu->addChild(btn);
    }

    btnMenu->updateLayout();
    m_mainLayer->addChildAtPosition(btnMenu, Anchor::Bottom, {0, 28});

    return true;
}

int ReplyPopup::getRemainingChars() {
    return 100 - m_textInput->getString().length();
}

// ReplyUploadPopup (where the comment gets uploaded)
ReplyUploadPopup* ReplyUploadPopup::create(int levelID, const std::string& comment, ReplyPopup* replyPopup) {
    auto ret = new ReplyUploadPopup();
    if (ret->init(levelID, comment, replyPopup)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool ReplyUploadPopup::init(int levelID, const std::string& comment, ReplyPopup* replyPopup) {
    if (!Popup::init(200, 100, "GJ_square02.png")) return false;
    m_replyPopup = replyPopup;
    m_noElasticity = false;

    // Loading spinner
    m_loadingSpinner = LoadingSpinner::create(40);
    m_mainLayer->addChildAtPosition(m_loadingSpinner, Anchor::Center, {0, 11});

    // Label
    m_label = Label::create("Replying...", "chatFont.fnt");
    m_label->setScale(.8f);
    m_mainLayer->addChildAtPosition(m_label, Anchor::Bottom, {0, 24});

    auto glm = GameLevelManager::get();
    glm->m_commentUploadDelegate = this;
    glm->uploadLevelComment(levelID, comment, 0);
    return true;
}

void ReplyUploadPopup::onClose(CCObject* sender) {
    if (!m_finished) return;
    m_replyPopup->onClose(sender);
    Popup::onClose(sender);
}

void ReplyUploadPopup::commentUploadFinished(int parentID) {
    m_finished = true;

    m_loadingSpinner->setVisible(false);
    m_label->setText("Reply added!");

    // Checkmark
    auto checkmark = CCSprite::createWithSpriteFrameName("GJ_completesIcon_001.png");
    checkmark->setScale(1.5f);
    m_mainLayer->addChildAtPosition(checkmark, Anchor::Center, {0, 11});
}

void ReplyUploadPopup::commentUploadFailed(int parentID, CommentError errorType) {
    m_finished = true;

    m_loadingSpinner->setVisible(false);
    m_label->setText("Upload failed. Please try again later.");

    // X-mark
    auto xmark = CCSprite::createWithSpriteFrameName("GJ_deleteIcon_001.png");
    xmark->setScale(1.5f);
    m_mainLayer->addChildAtPosition(xmark, Anchor::Center, {0, 11});
}

ReplyUploadPopup::~ReplyUploadPopup() {
    auto glm = GameLevelManager::get();
    if (glm->m_commentUploadDelegate == this) {
        glm->m_commentUploadDelegate = nullptr;
    }
}
