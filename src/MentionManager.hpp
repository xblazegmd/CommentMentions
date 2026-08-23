#pragma once

#include <utils.hpp>
#include <CommentObject.hpp>

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

    std::deque<CommentObject> getPreviousMentions() const;
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
    inline bool isSelfMention(int accountID);

    bool isPrevious(const CommentObject& obj);
    void storePrevious(const CommentObject& obj);

    inline bool isCommentInappropriate(const std::string& comment);
    bool isBlacklisted(const std::string& username);

    std::vector<std::string> getBlacklistedAccounts();
};