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

$on_game(Loaded) {
    async::spawn([] -> arc::Future<> {
        // Get daily level
        auto req = web::WebRequest()
            .userAgent("")
            .bodyString("type=21&secret=" + SECRET)
            .timeout(std::chrono::seconds(10));

        auto res = co_await req.post(BOOMLINGS + "getGJLevels21.php");
        if (!res.ok() || res.string().isErr()) {
            log::error("Request failed: (status code: {})", res.code());
            Notification::create(fmt::format("Error: {}", res.code()), NotificationIcon::Error)->show();
            co_return;
        }

        std::string resStr = res.string().unwrap();
        auto resStrNum = utils::numFromString<int>(resStr);

        if (resStrNum.isOk() && resStrNum.unwrap() < 0) {
            log::error("Reqest failed: {}", resStr);
            Notification::create(fmt::format("Error: {}", resStr), NotificationIcon::Error)->show();
            co_return;
        }

        auto daily = string::split(string::split(resStr, "#")[0], "|")[0];
        auto dailyID = formatKV(daily, {{"1", "daily"}})["daily"];
        auto intDailyID = utils::numFromString<int>(dailyID);

        if (intDailyID.isErr()) {
            Notification::create(fmt::format("Error: {}", intDailyID.unwrapErr()), NotificationIcon::Error)->show();
            co_return;
        }

        g_mentionManager = std::make_shared<MentionManager>(intDailyID.unwrap());
        g_mentionManager->startListening();
    });
}