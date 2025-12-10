#include "formatReq.hpp"

#include <Geode/utils/string.hpp>
#include <string>
#include <unordered_map>

using namespace geode::utils;

namespace formatReq {
    StrMap stringToMap(std::string str, std::string sep) {
        auto parts = string::split(str, sep);

        StrMap product;
        for (int i = 0; i + 1 < parts.size(); i += 2) {
            product[parts[i]] = parts[i + 1];
        }

        return product;
    }

    StrMap formatMap(const StrMap& map, const StrMap& formatKeys) {
        StrMap product;

        for (const auto& [k, v] : map) {
            auto it = formatKeys.find(k);
            std::string nk = (it != formatKeys.end()) ? it->second : k;
            product[nk] = v;
        }

        return product;
    }

    /// Not to be confused with a Comment String (see `formatReq.formatCommentStr`)
    ///
    /// Comment Object format: CommentString:AuthorString
    std::unordered_map<std::string, StrMap> formatCommentObj(std::string str) {
        auto pieces = string::split(str, ":");
        std::unordered_map<std::string, StrMap> commentObj{
            { "commentStr", formatCommentStr(pieces[0]) },
            { "authorStr", formatAuthorStr(pieces[1]) }
        };
        return commentObj;
    }

    /// Not to be confused with a comment object (see `formatReq.formatCommentObj`)
    ///
    /// Comment String format: key~val~key~val
    StrMap formatCommentStr(std::string str) {
        StrMap keyMap{
            { "1", "levelID" },
            { "2", "comment" },
            { "3", "authorPlayerID" },
            { "4", "likes" },
            { "5", "dislikes" },
            { "6", "messageID" },
            { "7", "spam" },
            { "8", "authorAccID" },
            { "9", "age" },
            { "10", "percentage" },
            { "11", "modBadge" },
            { "12", "modColor" }
        };

        return formatMap(stringToMap(str, "~"), keyMap);
    }

    StrMap formatAuthorStr(std::string str) {
        StrMap keyMap{
            { "1", "username" },
            { "9", "icon" },
            { "10", "colorA" },
            { "11", "colorB" },
            { "14", "iconType" },
            { "15", "glow" },
            { "16", "accID" }
        };

        return formatMap(stringToMap(str, "~"), keyMap);
    }
}