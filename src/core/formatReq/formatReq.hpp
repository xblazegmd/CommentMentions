#pragma once

#include <string>
#include <unordered_map>

namespace formatReq {
    using StrMap = std::unordered_map<std::string, std::string>;

    StrMap stringToMap(std::string str, std::string sep);

    StrMap formatMap(const StrMap& map, const StrMap& formatKeys);

    std::unordered_map<std::string, StrMap> formatCommentObj(std::string str);

    StrMap formatCommentStr(std::string str);

    StrMap formatAuthorStr(std::string str);
}