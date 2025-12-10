#include "utils.hpp"

#include <Geode/Geode.hpp>
#include <Geode/Result.hpp>
#include <Geode/loader/Event.hpp>
#include <Geode/utils/general.hpp>
#include <Geode/utils/string.hpp>
#include <Geode/utils/web.hpp>
#include <chrono>
#include <string>
#include <vector>

using namespace geode;
using namespace geode::prelude;
using namespace geode::utils;

namespace CMutils {
    void notify(std::string title, std::string msg) {
        AchievementNotifier::sharedState()->notifyAchievement(
            title.c_str(),
            msg.c_str(),
            "accountBtn_messages_001.png",
            true
        );
    }

    LevelFetch::LevelFetch(LevelFetchTarget target) : m_target(target) {}

    void LevelFetch::fetchID(std::function<void(Result<int>)> callback) {
        std::string ftype = "21";
        if (m_target == LevelFetchTarget::Weekly) {
            ftype = "22";
        }

        std::string params = "type=" + ftype + "&secret=" + SECRET;

        auto req = web::WebRequest();
        req.userAgent("");
        req.timeout(std::chrono::seconds(10));
        req.bodyString(params);
            
        m_reqListener = std::make_shared<EventListener<web::WebTask>>();
        m_reqListener->bind([this, callback](web::WebTask::Event* ev) {
            if (web::WebResponse* res = ev->getValue()) {
                if (res->ok() && res->string().isOk()) {
                    log::info("Response: {}", res->string().unwrap());
                    callback(Ok(parseResponse(res->string().unwrap())));
                } else {
                    log::error("Error when fetching daily level ID ({}, response: {})", res->code(), res->string().unwrap());
                    callback(Err(std::to_string(res->code())));
                }
            } else if (web::WebProgress* prog = ev->getProgress()) {
                log::info("Progress: {}", prog->downloadProgress().value_or(0.f));
            } else if (ev->isCancelled()) {
                log::error("Request got CANCELLED for whatever reason ;-;");
            }
        });

        auto reqTask = req.post(BOOMLINGS + "getGJLevels21.php");
        m_reqListener->setFilter(reqTask);

        FLAlertLayer::create(
            "This is risky but",
            "This is proof the code worked",
            "Ok"
        )->show();
    }

    int LevelFetch::parseResponse(std::string res) {
        // split the response TWO TIMES (gosh)
        auto levels = string::split(res, "#");
        auto levelsSplit = string::split(levels[0], "|");
        auto dailyLevel = string::split(levelsSplit[0], ":");

        for (int i = 0; i < dailyLevel.size(); i += 2) {
            if (dailyLevel[i] == "1") {
                int dailyID = numFromString<int>(dailyLevel[i + 1]).unwrapOr(0); // TODO: Proper handling for errors
                return dailyID;
            }
        }
        return 0;
    }
}