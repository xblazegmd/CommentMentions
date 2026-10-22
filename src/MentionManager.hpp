#pragma once

#include <Geode/utils/async.hpp>

#include <core/UserMention.hpp>
#include <vector>

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
private:
    geode::async::TaskHolder<> m_pollTask;
    std::vector<Target> m_targets;

    arc::Future<> poll();

    arc::Future<geode::Result<>> searchLevelComments(int levelID);
    arc::Future<geode::Result<>> searchGroupChat(int threadID);

    bool containsMention(const std::string& message);
};
