#pragma once

#include <Geode/utils/StringMap.hpp>
#include <Geode/utils/async.hpp>
#include <string>
#include <vector>

class MentionManager {
public:
    MentionManager(std::vector<int> targets);
    ~MentionManager() = default;

    void startListening();
private:
    std::vector<int> m_targets;
    std::vector<std::string> m_aliases;
    geode::async::TaskHolder<> m_listenerTask;

    struct CommentObject {
        geode::utils::StringMap<std::string> comment;
        geode::utils::StringMap<std::string> author;
    };

    std::vector<CommentObject> m_mentions;
    std::vector<CommentObject> m_previousMentions;

    arc::Future<> commentListener();
    void onMention(CommentObject obj);
    bool containsMention(const std::string& str);
    bool isSelfMention(const std::string& str);

    bool isPrevious(CommentObject obj);
    void storePrevious(CommentObject obj);

    CommentObject formatCommentObj(const std::string& str);
};