#pragma once

#include <arc/future/Future.hpp>
#include <arc/sync/Mutex.hpp>
#include <Geode/utils/StringMap.hpp>
#include <Geode/utils/async.hpp>
#include <string>
#include <vector>

class MentionManager {
public:
    MentionManager() {}
    ~MentionManager() = default;

    static MentionManager* get();
    arc::Future<> setLevelIDs(std::vector<int> levelIDs);

    void start();
    void restart();
private:
    arc::Mutex<std::vector<int>> m_levelIDs;
    geode::async::TaskHolder<> m_watcher;

    struct CommentObject {
        geode::utils::StringMap<std::string> comment;
        geode::utils::StringMap<std::string> author;
    };

    std::vector<CommentObject> m_mentions;
    std::vector<CommentObject> m_previousMentions;

    arc::Future<> commentWatcher();
    void onMention(const CommentObject& obj);
    bool containsMention(const std::string& str);
    bool isSelfMention(const std::string& str);

    bool isPrevious(const CommentObject& obj);
    void storePrevious(const CommentObject& obj);

    std::vector<std::string> getAliases();

    CommentObject formatCommentObj(const std::string& str);
};