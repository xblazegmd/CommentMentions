#include "utils.hpp"

#include <Geode/Geode.hpp>
#include <Geode/loader/Event.hpp>
#include <Geode/utils/web.hpp>
#include <string>

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

    int LevelFetch::fetchID() {
        std::string ftype = "21";
        if (m_target == LevelFetchTarget::Weekly) {
            ftype = "22";
        }

        std::string params = "type=" + ftype + "&secret=" + SECRET;

        auto req = web::WebRequest()
            .userAgent("")
            .bodyString(params);
            
        m_reqListener = std::make_shared<EventListener<web::WebTask>>();
        m_reqListener->bind([this](web::WebTask::Event* ev) {
            if (web::WebResponse* res = ev->getValue()) {
                if (res->ok() && res->string().isOk()) {
                    notify(
                        "IT WORKED",
                        "This is proof it 100% worked"
                    );
                    log::info("Response: {}", res->string().unwrap());
                } else {
                    notify(
                        "It worked... BUT",
                        "The request failed (" + std::to_string(res->code()) + ")"
                    );
                    log::error("Error when fetching daily level ID ({}, response: {})", res->code(), res->string().unwrap());
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
        return 0;
    }
}