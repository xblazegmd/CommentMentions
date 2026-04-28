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
        Mod::get()->setSettingValue("aliases", "@everyone, " + string::toLower(username));
    }
}

$on_mod(DataSaved) {
    g_mentionManager->save();
}

enum class LevelType {
    Daily = 21,
    Weekly = 22,
    Event = 23
};

arc::Future<Result<int>> getSpecialID(LevelType type) {
    auto res = co_await xblazeapi::requestGDServers("getGJLevels21.php", fmt::format(
        "type={}&secret={}",
        static_cast<int>(type), xblazeapi::SECRET
    ));
    if (res.isErr()) {
        log::error("{}", res.unwrapErr());
        co_return Err("{}", res.unwrapErr());
    }

    auto daily = string::split(string::split(res.unwrap(), "#")[0], "|")[0];
    auto dailyID = formatKV(daily, {{"1", "daily"}})["daily"];
    auto intDailyID = utils::numFromString<int>(dailyID);

    if (intDailyID.isErr()) {
        co_return Err(intDailyID.unwrapErr());
    }

    co_return Ok(intDailyID.unwrap());
}


$on_game(Loaded) {
    if (!Mod::get()->setSavedValue("unset-everyone-popup", true)) {
        FLAlertLayer::create(
            "CommentMentions",
            "Tired of lots of <cl>@everyone</c> notifications? You can remove the <cl>@everyone</c> alias by going to the <co>mod's settings</c>\n<cy>However, I think it's more fun keeping it on /Xblaze</c>",
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

        std::vector<int> levelIDs{};

        // Get daily level
        if (Mod::get()->getSettingValue<bool>("daily-lvl")) {
            auto dailyID = co_await getSpecialID(LevelType::Daily);
            if (dailyID.isErr()) {
                notifyError(fmt::format("CommentMentions: Could not get daily level ID: {}", dailyID.unwrapErr()));
            } else {
                levelIDs.push_back(std::move(dailyID).unwrap());
            }
        }

        // Get weekly demon
        if (Mod::get()->getSettingValue<bool>("weekly-demon")) {
            auto weeklyID = co_await getSpecialID(LevelType::Weekly);
            if (weeklyID.isErr()) {
                notifyError(fmt::format("CommentMentions: Could not get weekly demon ID: {}", weeklyID.unwrapErr()));
            } else {
                levelIDs.push_back(std::move(weeklyID).unwrap());
            }
        }

        // Get event level
        if (Mod::get()->getSettingValue<bool>("event-lvl")) {
            auto eventID = co_await getSpecialID(LevelType::Event);
            if (eventID.isErr()) {
                notifyError(fmt::format("CommentMentions: Could not get event level ID: {}", eventID.unwrapErr()));
            } else {
                log::info("{}", eventID);
                levelIDs.push_back(std::move(eventID).unwrap());
            }
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

                levelIDs.push_back(std::move(idNum).unwrap());
            }
        }

        // Check if there's even any IDs
        if (levelIDs.empty()) {
            notifyError("CommentMentions: No IDs were given");
            co_return;
        }

        // Start tracking for mentions
        g_mentionManager = std::make_shared<MentionManager>(std::move(levelIDs));
        g_mentionManager->start();
    });
}