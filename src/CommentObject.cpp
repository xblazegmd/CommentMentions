#include "CommentObject.hpp"

#include <utils.hpp>

#include <Geode/utils/general.hpp>
#include <Geode/utils/StringMap.hpp>
#include <Geode/utils/string.hpp>
#include <string>

#include <xblazegmd.geode-api/include/XblazeAPI.hpp>

using namespace geode::prelude;

CommentObject CommentObject::fromString(const std::string &str) {
    auto split = string::split(str, ":");
    auto commentStr = xblazeapi::formatResponse(split[0], "~");
    auto authorStr = xblazeapi::formatResponse(split[1], "~");

    CommentObject ret;

    ret.comment = commentStr;
    ret.author = authorStr;

    ret.commentt = commentStr["2"];
    ret.age = commentStr["9"];
    ret.username = authorStr["1"];

    ret.messageID = stoicm(commentStr["6"], -1);
    ret.levelID = stoicm(commentStr["1"], -1);
    ret.userID = stoicm(commentStr["3"], -1);
    ret.accountID = stoicm(authorStr["16"], -1);
    ret.likes = stoicm(commentStr["4"], -1);
    ret.percent = stoicm(commentStr["10"], -1);
    ret.modBadge = stoicm(commentStr["11"], -1);
    ret.iconID = stoicm(authorStr["9"], 1); // default cube
    ret.color1 = stoicm(authorStr["10"], 0); // default primary color
    ret.color2 = stoicm(authorStr["11"], 3); // default secondary color
    ret.color3 = stoicm(authorStr["51"], 0);
    ret.iconType = stoicm(authorStr["14"], 0);

    ret.isSpam = static_cast<bool>(stoicm(commentStr["7"], 0));
    ret.glow = static_cast<bool>(stoicm(commentStr["15"], 0));

    return ret;
}