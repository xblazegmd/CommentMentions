#include <Geode/Geode.hpp>
#include <Geode/utils/async.hpp>
#include <Geode/modify/DailyLevelPage.hpp>

#include <MentionManager.hpp>
#include <optional>

using namespace geode::prelude;

class $modify(DLPHook, DailyLevelPage) {
    void levelDownloadFinished(GJGameLevel* level) {
        DailyLevelPage::levelDownloadFinished(level);

        int newID = level->m_levelID.value();
        switch (m_type) {
            case GJTimedLevelType::Daily:
                async::spawn(
                    MentionManager::get()->getDailyID(),
                    [this, newID](std::optional<int> levelID) {
                        if (this->isNew(levelID, newID)) {
                            log::info("New daily level found, reloading...");
                            async::spawn(MentionManager::get()->fetchDailyID());
                        }
                    }
                );
                break;
            case GJTimedLevelType::Weekly:
                async::spawn(
                    MentionManager::get()->getWeeklyID(),
                    [this, newID](std::optional<int> levelID) {
                        if (this->isNew(levelID, newID)) {
                            log::info("New weekly demon found, reloading...");
                            async::spawn(MentionManager::get()->fetchWeeklyID());
                        }
                    }
                );
                break;
            case GJTimedLevelType::Event:
                async::spawn(
                    MentionManager::get()->getEventID(),
                    [this, newID](std::optional<int> levelID) {
                        if (this->isNew(levelID, newID)) {
                            log::info("New event level found, reloading...");
                            async::spawn(MentionManager::get()->fetchEventID());
                        }
                    }
                );
                break;
        }
    }

    inline bool isNew(std::optional<int> currentID, int newID) {
        return currentID && *currentID != newID;
    }
};