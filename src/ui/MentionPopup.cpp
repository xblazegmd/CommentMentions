#include "MentionPopup.hpp"

#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/binding/GJComment.hpp>
#include <Geode/binding/GJUserScore.hpp>

#include <CommentObject.hpp>

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
    if (!Popup::init(420, 240, "GJ_square02.png")) return false;

    this->setTitle(fmt::format("Mention from @{}", obj.username));

    // TODO

    return true;
}

GJComment* MentionPopup::objToComment(const CommentObject& obj) {
    auto ret = GJComment::create();
    return ret;
}
