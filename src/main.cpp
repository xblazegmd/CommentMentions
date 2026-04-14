#include <arc/prelude.hpp>
#include <MentionManager.hpp>
#include <utils.hpp>

#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>
#include <Geode/utils/string.hpp>

#include <string>

#include <xblazegmd.geode-api/include/XblazeAPI.hpp>

using namespace geode::prelude;

/// 1: Daily level, 2: Weekly demon
arc::Future<Result<int>> getSpecialID(const std::string& type) {
    auto req = co_await xblazeapi::requestGDServers("getGJLevels21.php", fmt::format(
        "type=2{}&secret={}",
        type, xblazeapi::SECRET
    ));

    if (req.isErr()) {
        log::error("{}", req.unwrapErr());
        co_return Err("{}", req.unwrap());
    }

    auto res = req.unwrap();
    auto num = utils::numFromString<int>(res);

    if (num.isErr()) {
        log::error("{}", num.unwrapErr());
        co_return Err(num.unwrapErr());
    }

    co_return Ok(num.unwrap());
}

$execute {
    if (Mod::get()->setSavedValue("loaded-before", true)) {
        auto username = GJAccountManager::get()->m_username;
        Mod::get()->setSettingValue("aliases", "everyone, " + string::toLower(username));
    }
}

$on_game(Loaded) {
    async::spawn([] -> arc::Future<> {
        auto doWeHaveInternet = co_await doWeTrulyHaveInternet();
        if (!doWeHaveInternet) {
            notifyError("CommentMentions: No internet connection!");
            notifyError("Please check your internet connection and restart the game");
            co_return; // We inmediatly abort any comment tracking (that's why we tell the user to restart their game)
        }

        std::vector<int> levels{};

        // Get daily level
        if (Mod::get()->getSettingValue<bool>("daily-lvl")) {
            auto dailyID = co_await getSpecialID("1");
            if (dailyID.isErr()) {
                notifyError(fmt::format("CommentMentions: Could not get daily level ID: {}", dailyID.unwrapErr()));
            } else {
                levels.push_back(std::move(dailyID).unwrap());
            }
        }

        // Get weekly demon
        if (Mod::get()->getSettingValue<bool>("weekly-demon")) {
            auto weeklyID = co_await getSpecialID("2");
            if (weeklyID.isErr()) {
                notifyError(fmt::format("CommentMentions: Could not get weekly demon ID: {}", weeklyID.unwrapErr()));
            } else {
                levels.push_back(std::move(weeklyID).unwrap());
            }
        }

        // Get custom IDs
        if (Mod::get()->getSettingValue<bool>("use-custom-ids")) {
            auto customIDs = Mod::get()->getSettingValue<std::string>("fixed-ids");
            auto ids = string::split(customIDs, ",");

            for (const auto& id : ids) {
                auto idNum = utils::numFromString<int>(string::trim(id));
                if (idNum.isErr()) {
                    log::error("Error converting ID {} to number: {}", id, idNum.unwrapErr());
                    continue;
                }

                levels.push_back(std::move(idNum).unwrap());
            }
        }

        // Check if there's even any IDs
        if (levels.empty()) {
            notifyError("CommentMentions: No IDs were given");
            co_return;
        }

        // Start tracking
        auto mentionManager = MentionManager::get();
        co_await mentionManager->setLevelIDs(std::move(levels));
        mentionManager->start();
    });
}