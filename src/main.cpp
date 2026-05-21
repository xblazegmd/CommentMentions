#include <MentionManager.hpp>
#include <arc/prelude.hpp>
#include <utils.hpp>
#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>
#include <Geode/utils/string.hpp>

#include <chrono>
#include <memory>
#include <string>

#include <xblazegmd.geode-api/include/XblazeAPI.hpp>

using namespace geode::prelude;

static std::shared_ptr<MentionManager> g_mentionManager;

$execute {
    if (!Mod::get()->setSavedValue("loaded-before", true)) {
        auto username = GJAccountManager::get()->m_username;
        Mod::get()->setSettingValue("aliases", string::toLower(username));
    }

    listenForSettingChanges<bool>("daily-lvl", [](bool enabled) {
        log::debug("'Track Daily Level' setting was toggled, reloading...");
        if (!g_mentionManager) return;
        if (enabled) {
            async::spawn(g_mentionManager->fetchDailyID());
        } else {
            g_mentionManager->disableDailyID();
        }
    });

    listenForSettingChanges<bool>("weekly-demon", [](bool enabled) {
        log::debug("'Track Weekly Demon' setting was toggled, reloading...");
        if (!g_mentionManager) return;
        if (enabled) {
            async::spawn(g_mentionManager->fetchWeeklyID());
        } else {
            g_mentionManager->disableWeeklyID();
        }
    });

    listenForSettingChanges<bool>("event-lvl", [](bool enabled) {
        log::debug("'Track Event Level' setting was toggled, reloading...");
        if (!g_mentionManager) return;
        if (enabled) {
            async::spawn(g_mentionManager->fetchEventID());
        } else {
            g_mentionManager->disableEventID();
        }
    });

    listenForSettingChanges<bool>("use-custom-ids", [](bool enabled) {
        log::debug("'Track Custom Levels' setting was toggled, reloading custom IDs...");
        if (!g_mentionManager) return;
        async::spawn(g_mentionManager->loadCustomIDs());
    });

    listenForSettingChanges<std::string>("custom-ids", [](std::string) {
        log::debug("Custom IDs were updated, reloading...");
        if (!g_mentionManager) return;
        async::spawn(g_mentionManager->loadCustomIDs());
    });
}

$on_mod(DataSaved) {
    g_mentionManager->save();
}

$on_game(Loaded) {
    if (!Mod::get()->setSavedValue("imsorry-popup", true)) {
        FLAlertLayer::create(
            "CommentMentions",
            "I'm <cb>sorry</c> for any <co>inconveniences</c> causes by ppl spamming <cg>@everyone</c> with the mod. I am <cl>trying</c> my best to fix it.\n<cr>DO NOT</c> use the <cg>@everyone</c> tag to <cs>annoy others</c>. It is <co>not funny</c>, and just look what happened to... <cy>actually nvm I can't namedrop them if I want my mod to be approved mb /Xblaze</c>",
            "OK"
        )->show();
    }

    async::spawn([] -> arc::Future<> {
        // Internet check
        if (!co_await xblazeapi::doWeHaveInternet()) {
            log::error("No internet connection!");
            notifyError("CommentMentions: No internet connection!\nPlease verify your internet connection and restart the game");
            co_return;
        }

        g_mentionManager = std::make_shared<MentionManager>();

        // Get daily level
        if (Mod::get()->getSettingValue<bool>("daily-lvl")) {
            co_await g_mentionManager->fetchDailyID();
        }

        // Get weekly demon
        if (Mod::get()->getSettingValue<bool>("weekly-demon")) {
            co_await g_mentionManager->fetchWeeklyID();
        }

        // Get event level
        if (Mod::get()->getSettingValue<bool>("event-lvl")) {
            co_await g_mentionManager->fetchEventID();
        }

        // Get custom IDs
        if (Mod::get()->getSettingValue<bool>("use-custom-ids")) {
            co_await g_mentionManager->loadCustomIDs();
        }

        // Start tracking for mentions
        g_mentionManager->start();
    });
}