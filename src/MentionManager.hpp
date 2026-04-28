#pragma once

#include <Geode/utils/StringMap.hpp>
#include <Geode/utils/async.hpp>
#include <matjson.hpp>
#include <matjson/std.hpp>
#include <string>
#include <vector>

struct CommentObject {
    geode::utils::StringMap<std::string> comment;
    geode::utils::StringMap<std::string> author;
};

template <>
struct matjson::Serialize<CommentObject> {
    static geode::Result<CommentObject> fromJson(const matjson::Value& val) {
        GEODE_UNWRAP_INTO(auto comment, val["comment"].as<geode::utils::StringMap<std::string>>());
        GEODE_UNWRAP_INTO(auto author, val["author"].as<geode::utils::StringMap<std::string>>());
        return geode::Ok(CommentObject{comment, author});
    }

    static matjson::Value toJson(const CommentObject& obj) {
        auto val = matjson::Value();
        val["comment"] = obj.comment;
        val["author"] = obj.author;
        return val;
    }
};

class MentionManager {
public:
    MentionManager(std::vector<int> levelIDs);
    ~MentionManager() = default;

    void start();
    void cleanup();
private:
    std::vector<int> m_levelIDs;
    geode::async::TaskHolder<> m_watcher;

    std::vector<CommentObject> m_mentions;
    std::vector<CommentObject> m_previousMentions;

    arc::Future<> commentWatcher();
    void onMention(const CommentObject& obj);
    bool containsMention(const std::string& str);
    bool isSelfMention(const std::string& str);

    bool isPrevious(const CommentObject& obj);
    void storePrevious(const CommentObject& obj);

    bool isCommentInappropriate(const std::string& comment);

    bool isBlacklisted(const std::string& username);

    std::vector<std::string> getAliases();
    std::vector<std::string> getBlacklistedAccounts();

    CommentObject formatCommentObj(const std::string& str);
};