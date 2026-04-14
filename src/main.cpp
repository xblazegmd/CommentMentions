#include "Geode/utils/general.hpp"
#include <arc/prelude.hpp>
#include <MentionManager.hpp>
#include <utils.hpp>

#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>
#include <Geode/utils/string.hpp>

#include <chrono>
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

$on_game(Loaded) {
    async::spawn([] -> arc::Future<> {
        // Internet check
        auto checkReq = web::WebRequest()
            .userAgent("GeometryDash/2.2081 CommentMentions/v1.0.0-beta.2")
            .timeout(std::chrono::seconds(10));

        auto check = co_await checkReq.get("https://www.google.com");
        if (!check.ok()) {
            log::error("No internet connection!");
            xblazeapi::quickErrorNotificationTS("CommentMentions: No internet connection!");
            co_return;
        }

        std::vector<int> targets{};

        // Get daily level
        if (Mod::get()->getSettingValue<bool>("daily-lvl")) {
            auto dailyID = co_await getSpecialID("1");
            if (dailyID.isErr()) {
                xblazeapi::quickErrorNotificationTS(fmt::format("CommentMentions: Could not get daily level ID: {}", dailyID.unwrapErr()));
            } else {
                targets.push_back(std::move(dailyID).unwrap());
            }
        }

        // Get weekly demon
        if (Mod::get()->getSettingValue<bool>("weekly-demon")) {
            auto weeklyID = co_await getSpecialID("2");
            if (weeklyID.isErr()) {
                xblazeapi::quickErrorNotificationTS(fmt::format("CommentMentions: Could not get weekly demon ID: {}", weeklyID.unwrapErr()));
            } else {
                targets.push_back(std::move(weeklyID).unwrap());
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

                targets.push_back(std::move(idNum).unwrap());
            }
        }

        // Check if there's even any IDs
        if (targets.empty()) {
            xblazeapi::quickErrorNotificationTS("CommentMentions: No IDs were given");
            co_return;
        }

        // Start tracking
        auto mentionManager = MentionManager::get();
        co_await mentionManager->setLevelIDs(std::move(targets));
        mentionManager->start();
    });
}