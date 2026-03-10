#include "Geode/ui/Notification.hpp"
#include <arc/prelude.hpp>
#include <MentionManager.hpp>
#include <utils.hpp>
#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>
#include <Geode/utils/string.hpp>
#include <chrono>
#include <memory>
#include <string>

using namespace geode::prelude;

static std::shared_ptr<MentionManager> g_mentionManager;

/// 1: Daily level, 2: Weekly demon
arc::Future<Result<int>> getSpecialID(const std::string& type) {
    auto req = web::WebRequest()
        .userAgent("")
        .bodyString("type=2" + type + "&secret=" + SECRET)
        .timeout(std::chrono::seconds(10));

    auto res = co_await req.post(BOOMLINGS + "getGJLevels21.php");
    if (!res.ok() || res.string().isErr()) {
        log::error("Request failed (status code {})", res.code());
        co_return Err("Request failed (status code {})", res.code());
    }

    std::string resStr = res.string().unwrap();
    auto resStrNum = utils::numFromString<int>(resStr);

    if (resStrNum.isOk() && resStrNum.unwrap() < 0) {
        log::error("Request to GD servers failed (error code {})", resStr);
        co_return Err("Request failed (error code {})", resStr);
    }

    auto daily = string::split(string::split(resStr, "#")[0], "|")[0];
    auto dailyID = formatKV(daily, {{"1", "daily"}})["daily"];
    auto intDailyID = utils::numFromString<int>(dailyID);

    if (intDailyID.isErr()) {
        co_return Err(intDailyID.unwrapErr());
    }

    co_return Ok(intDailyID.unwrap());
}

void showErrorNotification(const std::string& msg) {
    geode::queueInMainThread([msg] {
        Notification::create(msg, NotificationIcon::Error)->show();
    });
}

$on_game(Loaded) {
    async::spawn([] -> arc::Future<> {
        // Internet check
        auto checkReq = web::WebRequest()
            .userAgent("Geometry Dash! (internet check)")
            .timeout(std::chrono::seconds(10));

        auto check = co_await checkReq.get("https://www.google.com");
        if (!check.ok()) {
            log::error("No internet connection!");
            showErrorNotification("No internet connection!");
            co_return;
        }

        std::vector<int> targets{};

        // Get daily level
        if (Mod::get()->getSettingValue<bool>("daily-lvl")) {
            auto dailyID = co_await getSpecialID("1");
            if (dailyID.isErr()) {
                showErrorNotification(fmt::format("Could not get daily level ID: {}", dailyID.unwrapErr()));
            } else {
                targets.push_back(std::move(dailyID).unwrap());
            }
        }

        // Get weekly demon
        if (Mod::get()->getSettingValue<bool>("weekly-demon")) {
            auto weeklyID = co_await getSpecialID("2");
            if (weeklyID.isErr()) {
                showErrorNotification(fmt::format("Could not get weekly demon ID: {}", weeklyID.unwrapErr()));
            } else {
                targets.push_back(std::move(weeklyID).unwrap());
            }
        }

        // Get fixed IDs
        if (Mod::get()->getSettingValue<bool>("use-fixed-ids")) {
            auto fixedIDs = Mod::get()->getSettingValue<std::string>("fixed-ids");
            auto ids = string::split(fixedIDs, ",");

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
            showErrorNotification("No IDs were given");
            co_return;
        }

        // Start listener
        g_mentionManager = std::make_shared<MentionManager>(std::move(targets));
        g_mentionManager->startListening();
    });
}