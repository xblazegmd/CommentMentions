#include "MentionPopup.hpp"

#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/TextArea.hpp>
#include <Geode/ui/Layout.hpp>

#include <Geode/binding/GJSearchObject.hpp>
#include <Geode/binding/LevelBrowserLayer.hpp>

#include <core/CommentObject.hpp>
#include <utils.hpp>

#include <algorithm>
#include <regex>

using namespace geode::prelude;

MentionPopup* MentionPopup::create(const CommentObject& obj) {
    auto ret = new MentionPopup();
    if (ret->init(obj)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool MentionPopup::init(const CommentObject& obj) {
    if (!Popup::init({390, 220}, "GJ_square02.png")) return false;
    m_obj = obj;
    m_noElasticity = false;

    this->setTitle(fmt::format("Mention from @{}", m_obj.username), "goldFont.fnt", .7f, 22);

    // Comment area
    auto commentArea = CCLayer::create();
    commentArea->setID("comment-area"_spr);

    // Background
    auto bg = NineSlice::create("square02_small.png");
    bg->setContentSize(m_commentAreaSize);
    bg->setOpacity(70);
    commentArea->addChildAtPosition(bg, Anchor::Center);

    // Text Area
    auto textArea = RichTextArea::create(this->getCommentWithHighlight());
    textArea->setWidth(m_commentAreaSize.width - 30);
    textArea->setWrappingMode(WrappingMode::CUTOFF_WRAP);
    textArea->setAlignment(cocos2d::kCCTextAlignmentCenter);
    commentArea->addChildAtPosition(textArea, Anchor::Center);

    m_mainLayer->addChildAtPosition(commentArea, Anchor::Center, {0, 3});

    // Buttons
    auto btnMenu = CCMenu::create();
    btnMenu->setID("buttons"_spr);
    btnMenu->setContentSize({ m_commentAreaSize.width - 10, 20});
    btnMenu->setAnchorPoint({.5f, .5f});
    btnMenu->setLayout(
        RowLayout::create()
            ->setAutoScale(false)
            ->setGap(20)
    );

    // View Level
    auto viewLvlSpr = ButtonSprite::create("View Level", "bigFont.fnt", "GJ_button_01.png");
    viewLvlSpr->setScale(.6f);
    auto viewLvlBtn = Button::createWithNode(viewLvlSpr, [this](Button*) {
        auto searchObj = GJSearchObject::create(SearchType::Type19, fmt::format("{}&gameVersion=22", m_obj.levelID));
        auto scene = LevelBrowserLayer::scene(searchObj);
        CCDirector::get()->replaceScene(CCTransitionFade::create(.5f, scene));
    });
    btnMenu->addChild(viewLvlBtn);

    // Hide User
    auto hideUserSpr = ButtonSprite::create("Hide User", "bigFont.fnt", "GJ_button_06.png");
    hideUserSpr->setScale(.6f);
    auto hideUserBtn = Button::createWithNode(hideUserSpr, [this](Button*) {
        geode::createQuickPopup(
            "Hide User",
            fmt::format("Are you sure you want to <cp>hide</c> user <cy>@{}</c>? <cj>(mentions from that user will be ignored)</c>", m_obj.username),
            "No", "Yes",
            [this](auto, bool btn) {
                if (!btn) return;

                auto hiddenUsers = getListSetting("user-blacklist");
                if (std::ranges::find(hiddenUsers, m_obj.username) != hiddenUsers.end()) {
                    FLAlertLayer::create(
                        "Error",
                        fmt::format("User <cy>@{}</c> is <co>already hidden</c>", m_obj.username).c_str(),
                        "OK"
                    )->show();
                    return;
                }

                hiddenUsers.push_back(m_obj.username);
                setListSetting("user-blacklist", hiddenUsers);

                FLAlertLayer::create(
                    "Hidden",
                    fmt::format("User <cy>@{}</c> was hidden. Any upcoming mentions from them will be ignored", m_obj.username).c_str(),
                    "OK"
                )->show();
                this->onClose(nullptr);
            }
        );
    });
    btnMenu->addChild(hideUserBtn);

    btnMenu->updateLayout();
    m_mainLayer->addChildAtPosition(btnMenu, Anchor::Bottom, {0, 27});

    return true;
}

std::string MentionPopup::getCommentWithHighlight() {
    std::string ret;

    std::sregex_iterator begin(m_obj.commentt.begin(), m_obj.commentt.end(), m_mentionRegex);
    std::sregex_iterator end;

    size_t last = 0;
    for (auto it = begin; it != end; ++it) {
        auto match = *it;
        ret += m_obj.commentt.substr(last, match.position() - last);
        ret += "<color=#96FFFF>";
        ret += match.str();
        ret += "</color>";
        last = match.position() + match.length();
    }

    ret += m_obj.commentt.substr(last);
    return ret;
}
