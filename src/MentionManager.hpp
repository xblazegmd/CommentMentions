#pragma once

#include <Geode/utils/async.hpp>

#include <core/UserMention.hpp>

#include <vector>
#include <deque>

enum class TargetType {
    Level,
    GroupChat
};

struct Target {
    TargetType type;
    int id;
};

class MentionManager {
public:
    static MentionManager* get();

    arc::Future<> getLevels();
    arc::Future<> getGroupChats();

    void start();
    void stop();
private:
    geode::async::TaskHolder<> m_pollTask;
    std::vector<Target> m_targets;
    std::deque<UserMention> m_mentions;
    std::vector<UserMention> m_queue;

    arc::Future<> poll();

    arc::Future<geode::Result<>> searchLevelComments(int levelID);
    arc::Future<geode::Result<>> searchGroupChat(int threadID);

    bool containsMention(const std::string& message);

    bool mentionExists(const UserMention& mention);
    void addMention(const UserMention& mention);

    void notifyMention(const UserMention& mention);
    void notifyMentions(int mentions);
    void pushNotification(const std::string& title, const std::string& message);
};
