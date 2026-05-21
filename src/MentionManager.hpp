#pragma once

#include <arc/sync/Mutex.hpp>
#include <utils.hpp>
#include <Geode/utils/StringMap.hpp>
#include <Geode/utils/async.hpp>

#include <deque>
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
    MentionManager();
    ~MentionManager() = default;

    void start();
    void save();

    arc::Future<> addLevelID(int levelID);
    arc::Future<> fetchSpecialID(LevelType type);
private:
    arc::Mutex<std::vector<int>> m_levelIDs;
    geode::async::TaskHolder<> m_watcher;

    std::vector<std::string> m_aliases;
    std::vector<CommentObject> m_mentions;
    std::deque<CommentObject> m_previousMentions;

    arc::Future<> commentWatcher();
    void onMention(const CommentObject& obj);
    void onMentionCompressed(int amount);
    void showNotification(const std::string& title, const std::string& msg);

    bool containsMention(const std::string& str);
    bool isSelfMention(const std::string& str);
    void updateAliases();

    bool isPrevious(const CommentObject& obj);
    void storePrevious(const CommentObject& obj);

    inline bool isCommentInappropriate(const std::string& comment);
    bool isBlacklisted(const std::string& username);

    std::vector<std::string> getBlacklistedAccounts();

    CommentObject formatCommentObj(const std::string& str);
};