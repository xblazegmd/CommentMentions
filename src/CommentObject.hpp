#pragma once

#include <utils.hpp>

#include <string>

struct CommentObject {
    std::unordered_map<std::string, std::string> comment;
    std::unordered_map<std::string, std::string> author;

    std::string commentt;
    std::string age;
    std::string username;

    int messageID;
    int levelID;
    int userID;
    int accountID;
    int likes;
    int percent;
    int modBadge;
    int iconID;
    int color1;
    int color2;
    int color3;
    int iconType;

    bool isSpam;
    bool glow;

    static CommentObject fromString(const std::string& str);
};


template <>
struct matjson::Serialize<CommentObject> {
    static geode::Result<CommentObject> fromJson(const matjson::Value& val) {
        GEODE_UNWRAP_INTO(auto comment, val["comment"].as<std::unordered_map<std::string, std::string>>());
        GEODE_UNWRAP_INTO(auto author, val["author"].as<std::unordered_map<std::string, std::string>>());

        // Backwards compatibility
        if (!val.contains("commentt")) {
            return geode::Ok(CommentObject{
                comment,
                author,
                comment["comment"],
                comment["age"],
                author["userName"],
                stoicm(comment["messageID"], -1),
                stoicm(comment["levelID"], -1),
                stoicm(comment["authorPlayerID"], -1),
                stoicm(author["accountID"], -1),
                stoicm(comment["likes"], -1),
                stoicm(comment["percent"], -1),
                stoicm(comment["modBadge"], -1),
                stoicm(author["icon"], 1),
                stoicm(author["color1"], 0),
                stoicm(author["color2"], 3),
                stoicm(author["iconType"], 0),
                static_cast<bool>(stoicm(comment["spam"], 0)),
                static_cast<bool>(stoicm(author["glow"], 0)),
            });
        }

        GEODE_UNWRAP_INTO(auto commentt, val["commentt"].asString());
        GEODE_UNWRAP_INTO(auto age, val["age"].asString());
        GEODE_UNWRAP_INTO(auto username, val["username"].asString());

        GEODE_UNWRAP_INTO(int messageID, val["messageID"].asInt());
        GEODE_UNWRAP_INTO(int levelID, val["levelID"].asInt());
        GEODE_UNWRAP_INTO(int userID, val["userID"].asInt());
        GEODE_UNWRAP_INTO(int accountID, val["accountID"].asInt());
        GEODE_UNWRAP_INTO(int likes, val["likes"].asInt());
        GEODE_UNWRAP_INTO(int percent, val["percent"].asInt());
        GEODE_UNWRAP_INTO(int modBadge, val["modBadge"].asInt());
        GEODE_UNWRAP_INTO(int iconID, val["iconID"].asInt());
        GEODE_UNWRAP_INTO(int color1, val["color1"].asInt());
        GEODE_UNWRAP_INTO(int color2, val["color2"].asInt());
        GEODE_UNWRAP_INTO(int color3, val["color3"].asInt());
        GEODE_UNWRAP_INTO(int iconType, val["iconType"].asInt());

        GEODE_UNWRAP_INTO(auto isSpam, val["isSpam"].asBool());
        GEODE_UNWRAP_INTO(auto glow, val["glow"].asBool());

        return geode::Ok(CommentObject{
            comment,
            author,
            commentt,
            age,
            username,
            messageID,
            levelID,
            userID,
            accountID,
            likes,
            percent,
            modBadge,
            iconID,
            color1,
            color2,
            color3,
            iconType,
            isSpam,
            glow
        });
    }

    static matjson::Value toJson(const CommentObject& obj) {
        auto val = matjson::Value();

        val["comment"] = obj.comment;
        val["author"] = obj.author;

        val["commentt"] = obj.commentt;
        val["age"] = obj.age;
        val["username"] = obj.username;

        val["messageID"] = obj.messageID;
        val["levelID"] = obj.levelID;
        val["userID"] = obj.userID;
        val["accountID"] = obj.accountID;
        val["likes"] = obj.likes;
        val["percent"] = obj.percent;
        val["modBadge"] = obj.modBadge;
        val["iconID"] = obj.iconID;
        val["color1"] = obj.color1;
        val["color2"] = obj.color2;
        val["color3"] = obj.color3;
        val["iconType"] = obj.iconType;

        val["isSpam"] = obj.isSpam;
        val["glow"] = obj.glow;

        return val;
    }
};
