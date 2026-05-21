#include <Geode/Geode.hpp>
#include <Geode/Enums.hpp>
#include <Geode/utils/async.hpp>
#include <Geode/modify/DailyLevelPage.hpp>

#include <MentionManager.hpp>

using namespace geode::prelude;

class $modify(DLPHook, DailyLevelPage) {
    void createDailyNode(GJGameLevel* level, bool instant, float delay, bool isNew) {
        DailyLevelPage::createDailyNode(level, instant, delay, isNew);
        if (isNew) {
            switch (m_type) {
                case GJTimedLevelType::Daily:
                    if (!Mod::get()->getSettingValue<bool>("daily-lvl")) return;

                    log::info("New daily level found, reloading...");
                    async::spawn(MentionManager::get()->fetchDailyID());
                    break;
                case GJTimedLevelType::Weekly:
                    if (!Mod::get()->getSettingValue<bool>("weekly-demon")) return;

                    log::info("New weekly demon found, reloading...");
                    async::spawn(MentionManager::get()->fetchWeeklyID());
                    break;
                case GJTimedLevelType::Event:
                    if (!Mod::get()->getSettingValue<bool>("event-lvl")) return;

                    log::info("New event level found, reloading...");
                    async::spawn(MentionManager::get()->fetchEventID());
                    break;
            }
        }
    }
};