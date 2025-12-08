#include "utils.hpp"

#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>
#include <chrono>

using namespace geode;
using namespace geode::prelude;
using namespace geode::utils;

namespace CMentions::utils {
    Result<int> getDailyLevel() {
        geode::log::debug("called getDailyLevel");
        web::WebRequest req = web::WebRequest();
        req.userAgent("");
        req.timeout(std::chrono::seconds(10));

        geode::log::debug("defined web request");
    
        auto paramResult = matjson::parse(R"(
            {
                "type": 21,
                "secret": "Wmfd2893gb7"
            }
        )");
        geode::log::debug("parsed param json, checking for errors...");
        if (paramResult.isErr()) {
            geode::log::error("In case you're wondering why the JSON failed to parse: {}", paramResult.unwrapErr());
            return Err("Could not parse param JSON, FIX YOUR STUPID CODE XBLAZE");
        }
    
        geode::log::debug("no errors at all");
        auto params = paramResult.unwrap();
        req.bodyJSON(params);
        geode::log::debug("added parameters to body");

        auto task = req.post(BOOMLINGS + "getGJLevels21.php");
        geode::log::debug("made task");
        task.listen([](web::WebResponse *res) {
            if (!res || !res->ok()) {
                if (res->code() == 403) {
                    geode::log::error("Request to {} returned a 403 error", BOOMLINGS + "getGJLevels21.php");
                } else {
                    geode::log::error("Failed to get daily level info (status code: {})", res->code());
                }
                return Err("Failed to get daily level info (status code: {})", res->code());
            }
            geode::log::debug("response's ok");
            std::string text = res->string().unwrapOr("100");

            // FLAlertLayer::create(
            //     "Daily level ID",
            //     "The daily level id SHOULD BE " + text,
            //     "Ok ig"
            // )->show();
        });
        
        geode::log::debug("everything went 'fine'");
        return Ok(1);
    }
}