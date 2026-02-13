#pragma once

#include <Geode/Result.hpp>
#include <Geode/utils/general.hpp>
#include <arc/future/Future.hpp>
#include <string>

using namespace geode::prelude;

namespace CommentMentions {
    const std::string BOOMLINGS = "http://www.boomlings.com/database/";
    const std::string SECRET = "Wmfd2893gb7";

    struct CommentObject {
        utils::StringMap<std::string> comment;
        utils::StringMap<std::string> author;
    };

    utils::StringMap<std::string> formatReq(
        const std::string& str,
        const std::string& sep,
        const utils::StringMap<std::string>& formatMap
    );
    CommentObject formatCommentObj(const std::string& str);
    arc::Future<Result<int>> getSpecialID(const std::string& type);

    bool stringIsOk(Result<std::string> str);
    bool containsWord(std::string const& text, std::string const& contains);
    std::string cleanString(std::string const& text);
    bool isAlnum(char c);
}