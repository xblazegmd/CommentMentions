#include <MentionManager.hpp>
#include <utils.hpp>

#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>
#include <Geode/utils/string.hpp>

#include <arc/prelude.hpp>
#include <string>

#include <xblazegmd.geode-api/include/XblazeAPI.hpp>

using namespace geode::prelude;

$execute {
    if (!Mod::get()->setSavedValue("loaded-before", true)) {
        auto username = GJAccountManager::get()->m_username;
        Mod::get()->setSettingValue("aliases", string::toLower(username));
    }

    listenForSettingChanges<bool>("daily-lvl", [](bool enabled) {
        log::info("'Track Daily Level' setting was toggled, reloading...");
        if (enabled) {
            async::spawn(MentionManager::get()->fetchDailyID());
        } else {
            MentionManager::get()->disableDailyID();
        }
    });

    listenForSettingChanges<bool>("weekly-demon", [](bool enabled) {
        log::info("'Track Weekly Demon' setting was toggled, reloading...");
        if (enabled) {
            async::spawn(MentionManager::get()->fetchWeeklyID());
        } else {
            MentionManager::get()->disableWeeklyID();
        }
    });

    listenForSettingChanges<bool>("event-lvl", [](bool enabled) {
        log::info("'Track Event Level' setting was toggled, reloading...");
        if (enabled) {
            async::spawn(MentionManager::get()->fetchEventID());
        } else {
            MentionManager::get()->disableEventID();
        }
    });

    listenForSettingChanges<bool>("use-custom-ids", [](bool enabled) {
        log::info("'Track Custom Levels' setting was toggled, reloading custom IDs...");
        async::spawn(MentionManager::get()->loadCustomIDs());
    });

    listenForSettingChanges<std::string>("custom-ids", [](std::string) {
        log::info("Custom IDs were updated, reloading...");
        async::spawn(MentionManager::get()->loadCustomIDs());
    });
}

$on_mod(DataSaved) {
    MentionManager::get()->save();
}

$on_game(Loaded) {
    if (!Mod::get()->setSavedValue("random-ahh-popup", true)) {
        FLAlertLayer::create(
            "CommentMentions",
            "Thank you for using <cy>CommentMentions!</c>. I recommend you take a look at the mod's settings <cj>/Xblaze</c>",
            "OK"
        )->show();
    }

    async::spawn([] -> arc::Future<> {
        // Internet check
        bool connected;
        if (Mod::get()->getSettingValue<bool>("internal-internet-check")) {
            connected = co_await async::waitForMainThread([] {
                return GameToolbox::doWeHaveInternet();
            });
        } else {
            connected = co_await xblazeapi::doWeHaveInternet();
        }

        if (!connected) {
            log::error("No internet connection!");

            co_await pauseUntilWeHaveInternet(); // Wait until we have internet

            // Continue like usual
            log::error("Back online ;)");
        }

        auto mentionManager = MentionManager::get();

        // Get daily level
        if (Mod::get()->getSettingValue<bool>("daily-lvl")) {
            co_await mentionManager->fetchDailyID();
        }

        // Get weekly demon
        if (Mod::get()->getSettingValue<bool>("weekly-demon")) {
            co_await mentionManager->fetchWeeklyID();
        }

        // Get event level
        if (Mod::get()->getSettingValue<bool>("event-lvl")) {
            co_await mentionManager->fetchEventID();
        }

        // Get custom IDs
        if (Mod::get()->getSettingValue<bool>("use-custom-ids")) {
            co_await mentionManager->loadCustomIDs();
        }

        // Start tracking for mentions
        mentionManager->start();
    });
}