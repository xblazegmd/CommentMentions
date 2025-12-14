#include "utils.hpp"

#include <Geode/Geode.hpp>
#include <string>

using namespace geode::prelude;

namespace CMUtils {
    void notify(std::string title, std::string msg) {
        AchievementNotifier::sharedState()->notifyAchievement(
            title.c_str(),
            msg.c_str(),
            "accountBtn_messages_001.png",
            true
        );
    }
}