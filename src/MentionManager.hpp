#pragma once

#include <utils.hpp>

#include <Geode/utils/StringMap.hpp>
#include <Geode/utils/async.hpp>

#include <arc/future/Future.hpp>
#include <arc/sync/Mutex.hpp>
#include <deque>
#include <matjson.hpp>
#include <matjson/std.hpp>
#include <optional>
#include <regex>
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

    static MentionManager* get();

    void start();
    void save();

    arc::Future<> addLevelID(int levelID);
    arc::Future<> fetchSpecialID(LevelType type);

    arc::Future<> fetchDailyID();
    arc::Future<> fetchWeeklyID();
    arc::Future<> fetchEventID();
    arc::Future<> loadCustomIDs();

    arc::Future<std::optional<int>> getDailyID();
    arc::Future<std::optional<int>> getWeeklyID();
    arc::Future<std::optional<int>> getEventID();

    void disableDailyID();
    void disableWeeklyID();
    void disableEventID();
private:
    arc::Mutex<std::optional<int>> m_dailyID;
    arc::Mutex<std::optional<int>> m_weeklyID;
    arc::Mutex<std::optional<int>> m_eventID;
    arc::Mutex<std::vector<int>> m_customIDs;

    geode::async::TaskHolder<> m_watcher;

    std::regex m_aliasRegex;
    std::vector<CommentObject> m_mentions;
    std::deque<CommentObject> m_previousMentions;

    arc::Future<> commentWatcher();
    void onMention(const CommentObject& obj);
    void onMentionCompressed(int amount);
    void showNotification(const std::string& title, const std::string& msg);

    inline bool containsMention(const std::string& str);
    bool isSelfMention(const std::string& str);
    void updateAliases();

    bool isPrevious(const CommentObject& obj);
    void storePrevious(const CommentObject& obj);

    inline bool isCommentInappropriate(const std::string& comment);
    bool isBlacklisted(const std::string& username);

    std::vector<std::string> getBlacklistedAccounts();

    CommentObject formatCommentObj(const std::string& str);
};