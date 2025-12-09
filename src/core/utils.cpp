#include "utils.hpp"

#include <Geode/Geode.hpp>
#include <Geode/loader/Event.hpp>
#include <Geode/utils/coro.hpp>
#include <Geode/utils/web.hpp>
#include <string>

using namespace geode;
using namespace geode::prelude;
using namespace geode::utils;

namespace CMentions::utils {
    void notify(std::string title, std::string msg) {
        AchievementNotifier::sharedState()->notifyAchievement(
            title.c_str(),
            msg.c_str(),
            "accountBtn_messages_001.png",
            true
        );
    }
}