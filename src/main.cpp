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
        auto check = co_await web::WebRequest()
            .userAgent("Geometry Dash! (internet check)")
            .timeout(std::chrono::seconds(10))
            .get("http://connectivitycheck.gstatic.com/generate_204");

        if (!check.ok()) {
            log::error("No internet connection!");
            notifyError("CommentMentions: No internet connection!\nPlease check your internet connection and restart the game");
            co_return;
        }

        g_mentionManager = std::make_shared<MentionManager>();

        // Get daily level
        if (Mod::get()->getSettingValue<bool>("daily-lvl")) {
            co_await g_mentionManager->fetchSpecialID(LevelType::Daily);
        }

        // Get weekly demon
        if (Mod::get()->getSettingValue<bool>("weekly-demon")) {
            co_await g_mentionManager->fetchSpecialID(LevelType::Daily);
        }

        // Get event level
        if (Mod::get()->getSettingValue<bool>("event-lvl")) {
            co_await g_mentionManager->fetchSpecialID(LevelType::Daily);
        }

        // Get custom IDs
        if (Mod::get()->getSettingValue<bool>("use-custom-ids")) {
            auto customIDs = Mod::get()->getSettingValue<std::string>("custom-ids");
            auto ids = string::split(customIDs, ",");

            for (const auto& id : ids) {
                auto idNum = utils::numFromString<int>(string::trim(id));
                if (idNum.isErr()) {
                    log::error("Error converting ID {} to number: {}", id, idNum.unwrapErr());
                    continue;
                }
                co_await g_mentionManager->addLevelID(std::move(idNum).unwrap());
            }
        }

        // Start tracking for mentions
        g_mentionManager->start();
    });
}