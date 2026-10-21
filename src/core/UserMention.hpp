#pragma once

#include <Geode/Result.hpp>
#include <asp/time.hpp>
#include <matjson.hpp>
#include <string>

enum class MentionType {
    Comment,
    DM
};

struct UserMention {
    MentionType type;
    std::string message;

    std::string username;
    int accountID;

    int messageID;
    asp::Instant date;
};

template<>
struct matjson::Serialize<UserMention> {
    static geode::Result<UserMention> fromJson(const matjson::Value& val) {
        GEODE_UNWRAP_INTO(int type, val["type"].asInt());
        GEODE_UNWRAP_INTO(std::string message, val["message"].asString());
        GEODE_UNWRAP_INTO(std::string username, val["username"].asString());

        GEODE_UNWRAP_INTO(int messageID, val["messageID"].asInt());
        GEODE_UNWRAP_INTO(int accountID, val["accountID"].asInt());
        GEODE_UNWRAP_INTO(int date, val["date"].asInt());

        return geode::Ok(UserMention{
            static_cast<MentionType>(type),
            message,
            username,
            accountID,
            messageID,
            asp::Instant::fromRawNanos(date)
        });
    }

    static matjson::Value toJson(const UserMention& val) {
        return matjson::makeObject({
            { "type", static_cast<int>(val.type) },
            { "message", val.message },
            { "username", val.username },
            { "accountID", val.accountID },
            { "messageID", val.messageID },
            { "date", val.date.rawNanos() }
        });
    }
};
