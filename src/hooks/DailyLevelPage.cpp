#include <Geode/Geode.hpp>
#include <Geode/modify/DailyLevelPage.hpp>

using namespace geode::prelude;

class $modify(DLPHook, DailyLevelPage) {
    void levelDownloadFinished(GJGameLevel* level) {
        DailyLevelPage::levelDownloadFinished(level);
    }
};