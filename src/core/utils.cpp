#include "utils.hpp"

#include <Geode/Geode.hpp>
#include <Geode/loader/Event.hpp>
#include <Geode/utils/coro.hpp>
#include <Geode/utils/web.hpp>
#include <string>

using namespace geode;
using namespace geode::prelude;
using namespace geode::utils;

namespace CMentions::utils {
    void notify(std::string title, std::string msg) {
        AchievementNotifier::sharedState()->notifyAchievement(
            title.c_str(),
            msg.c_str(),
            "accountBtn_messages_001.png",
            true
        );
    }

    Result<int> getDailyLevel() {
        geode::log::debug("called getDailyLevel");
        EventListener<web::WebTask> m_listener;

        std::string params = "type=21&secret=" + SECRET;

        auto req = web::WebRequest().bodyString(params);
        auto task = req.post(BOOMLINGS + "getGJLevels21.php");
        m_listener.bind([](web::WebTask::Event* ev) {
            if (web::WebResponse* res = ev->getValue()) {
                geode::log::info("Request to {} finished!", BOOMLINGS + "getGJLevels21.php");
                geode::log::info("{}", res->string().unwrapOr("err"));
            } else if (web::WebProgress* prog = ev->getProgress()) {
                geode::log::info("Awaiting for response...");
            } else if (ev->isCancelled()) {
                geode::log::info("Task was cancelled :(");
            }
        });
        m_listener.setFilter(task);

        // bool succeded = true;
        // std::string errMsg = "";
        // web::WebRequest()
        //     .userAgent("")
        //     .timeout(std::chrono::seconds(10))
        //     .bodyString(params)
        //     .post(BOOMLINGS + "getGJLevels21.php")
        //     .listen([succeded, errMsg](web::WebResponse *res) mutable {
        //         if (!res || !res->ok()) {
        //             succeded = false;
        //             if (res->code() == 403) {
        //                 geode::log::error("Request to {} returned a 403 error", BOOMLINGS + "getGJLevels21.php");
        //                 errMsg = "Request to {} returned a 403 error", BOOMLINGS + "getGJLevels21.php";
        //             } else {
        //                 geode::log::error("Failed to get daily level info (status code: {})", res->code());
        //                 errMsg = "Failed to get daily level info (status code: " + std::to_string(res->code()) + ")";
        //             }
        //             return;
        //         }
        //         geode::log::debug("response's ok");
        //         std::string text = res->string().unwrapOr("100");
        //         geode::log::debug(text);

        //         // FLAlertLayer::create(
        //         //     "Daily level ID",
        //         //     "The daily level id SHOULD BE " + text,
        //         //     "Ok ig"
        //         // )->show();
        //     });
        
        // if (!succeded) {
        //     return Err(errMsg);
        // }
        
        // geode::log::debug("everything went 'fine'");
        // return Ok(1);
        return Ok(1);
    }
}