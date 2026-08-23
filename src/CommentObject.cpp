#include "CommentObject.hpp"

#include <Geode/utils/general.hpp>
#include <Geode/utils/StringMap.hpp>
#include <Geode/utils/string.hpp>
#include <string>

#include <xblazegmd.geode-api/include/XblazeAPI.hpp>

using namespace geode::prelude;

inline int stoi(const std::string& str, int def) {
    return utils::numFromString<int>(str).unwrapOr(def);
}

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

    ret.messageID = stoi(commentStr["6"], -1);
    ret.levelID = stoi(commentStr["1"], -1);
    ret.userID = stoi(commentStr["3"], -1);
    ret.accountID = stoi(authorStr["16"], -1);
    ret.likes = stoi(commentStr["4"], -1);
    ret.percent = stoi(commentStr["10"], -1);
    ret.modBadge = stoi(commentStr["11"], -1);
    ret.iconID = stoi(authorStr["9"], 1); // default cube
    ret.color1 = stoi(authorStr["10"], 0); // default primary color
    ret.color2 = stoi(authorStr["11"], 3); // default secondary color
    ret.color3 = stoi(authorStr["51"], 0);
    ret.iconType = stoi(authorStr["14"], 0);

    ret.isSpam = static_cast<bool>(stoi(commentStr["7"], 0));
    ret.glow = static_cast<bool>(stoi(commentStr["15"], 0));

    return ret;
}