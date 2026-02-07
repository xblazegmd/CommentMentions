#include "utils.hpp"

#include <Geode/Geode.hpp>
#include <Geode/Result.hpp>
#include <Geode/utils/general.hpp>
#include <Geode/utils/async.hpp>
#include <Geode/utils/base64.hpp>
#include <Geode/utils/string.hpp>
#include <Geode/utils/web.hpp>
#include <arc/future/Future.hpp>
#include <string>

using namespace geode::prelude;

namespace CMUtils {
    utils::StringMap<std::string> formatReq(
        const std::string& str,
        const std::string& sep,
        const utils::StringMap<std::string>& formatMap
    ) {
        auto parts = string::split(str, sep);

        utils::StringMap<std::string> raw;
        for (int i = 0; i + 1 < parts.size(); i += 2) {
            raw[parts[i]] = parts[i + 1];
        }

        utils::StringMap<std::string> ret;
        for (const auto& [k, v] : raw) {
            auto it = formatMap.find(k);
            std::string nk = it != formatMap.end() ? it->second : k;
            ret[nk] = v;
        }

        return ret;
    }

    CommentObject formatCommentObj(const std::string& str) {
        CommentObject ret;
        auto pieces = string::split(str, ":");

        ret.comment = formatReq(pieces[0], "~", {
            { "1", "levelID" },
            { "2", "comment" },
            { "3", "authorPlayerID" },
            { "4", "likes" },
            { "5", "dislikes" },
            { "6", "messageID" },
            { "7", "spam" },
            { "8", "authorAccountID" },
            { "9", "age" },
            { "10", "percent" },
            { "11", "modBadge" },
            { "12", "moderatorChatColor" },
        });
        ret.author = formatReq(pieces[1], "~", {
            { "1", "userName" },
            { "9", "icon" },
            { "10", "playerColor" },
            { "11", "playerColor2" },
            { "14", "iconType" },
            { "15", "glow" },
            { "16", "accountID" },
        });

        // Decode comment
        auto decoded = base64::decode(ret.comment["comment"], base64::Base64Variant::Url);
        if (decoded.isOk()) {
            auto bytes = decoded.unwrap();
            ret.comment["comment"] = std::string(bytes.begin(), bytes.end());
        } else log::error("Could not decode base64: {}", decoded.unwrapErr());

        return ret;
    }

    arc::Future<Result<int>> getSpecialID(const std::string& type) {
        auto req = web::WebRequest()
            .userAgent("")
            .timeout(std::chrono::seconds(10))
            .bodyString("type=" + type + "&secret=" + SECRET);
        auto res = co_await req.post(BOOMLINGS + "getGJLevels21.php");
		if (res.ok() && CMUtils::stringIsOk(res.string())) {
     		auto levels = string::split(res.string().unwrap(), "#");
     		auto levelsSplit = string::split(levels[0], "|");
     		auto dailyLevel = string::split(levelsSplit[0], ":");

     		for (int i = 0; i < dailyLevel.size(); i += 2) {
     		    if (dailyLevel[i] == "1") {
     		        int dailyID = numFromString<int>(dailyLevel[i + 1]).unwrapOr(0); // TODO: Fallback to fixed ID
                    co_return Ok(dailyID);
     		    }
     		}
		} else {
			Notification::create(
				"Could not fetch daily level ID",
				NotificationIcon::Error,
				2
			)->show();
            co_return Err("Coult not fetch daily level ID");
		}
    }

    bool stringIsOk(Result<std::string> str) {
        if (str.isErr()) return false;
        auto num = utils::numFromString<int>(str.unwrap());
        return num.isErr() || num.unwrap() > 0;
    }

    bool contains(std::string const& text, std::string const& contains) {
        auto pieces = string::split(text, " ");

        for (const auto& piece : pieces) {
            auto cleanPiece = cleanNameString(piece);
            if (string::toLower(cleanPiece) == contains) {
                return true;
            }
        }
        return false;
    }

    std::string cleanNameString(std::string name) {
        const auto len = name.size();
        if (len > 2 && string::endsWith(name, "'s")) {
            name.erase(len - 2);
        } else if (len > 1 && string::endsWith(name, "s'")) {
            name.erase(len - 1);
        }

        return cleanString(name);
    }

    std::string cleanString(std::string const& text) {
        std::string res;
        res.reserve(text.size());

        for (unsigned char c : text) {
            if (isAlnum(c)) {
                res.push_back(c);
            }
        }
        return res;
    }

    bool isAlnum(char c) {
        return (c >= 'a' && c <= 'z') ||
               (c >= 'A' && c <= 'Z') ||
               (c >= '0' && c <= '9');
    }
}