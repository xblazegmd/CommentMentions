#pragma once

#include <Geode/utils/async.hpp>

#include <core/UserMention.hpp>

#include <vector>
#include <deque>

class MentionManager {
public:
    static MentionManager* get();

    arc::Future<> getLevels();
    arc::Future<> getGroupChats();

    void start();
    void stop();
private:
    geode::async::TaskHolder<> m_pollTask;

    std::deque<UserMention> m_mentions;
    std::vector<UserMention> m_queue;

    std::vector<int> m_levels;
    int m_levelIdx;
    std::vector<int> m_groupChats;
    int m_groupChatIdx;

    arc::Future<> poll();

    arc::Future<geode::Result<>> searchLevelComments();
    arc::Future<geode::Result<>> searchGroupChatMessages();

    int nextLevel();
    int nextGroupChat();

    bool containsMention(const std::string& message);

    bool mentionExists(const UserMention& mention);
    void addMention(const UserMention& mention);

    void notifyMention(const UserMention& mention);
    void notifyMentions(int mentions);
    void pushNotification(const std::string& title, const std::string& message);
};
