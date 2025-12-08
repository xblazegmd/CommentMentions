#include "utils.hpp"

#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>
#include <chrono>

using namespace geode;
using namespace geode::prelude;
using namespace geode::utils;

namespace CMentions::utils {
    Result<int> getDailyLevel() {
        web::WebRequest req = web::WebRequest();
        req.userAgent("");
        req.timeout(std::chrono::seconds(10));
    
        auto paramResult = matjson::parse(R"(
            {
                "type": 21,
                "secret": "Wmfd2893gb7"
            }
        )");
        if (paramResult.isErr()) {
            geode::log::error("In case you're wondering why the JSON failed to parse: {}", paramResult.unwrapErr());
            return Err("Could not parse param JSON, FIX YOUR STUPID CODE XBLAZE");
        }
    
        auto params = paramResult.unwrap();
        req.bodyJSON(params);

        auto task = req.post(BOOMLINGS + "getGJLevels21.php");
        task.listen([](web::WebResponse *res) {
            if (!res || !res->ok()) {
                if (res->code() == 403) {
                    geode::log::error("Request to {} returned a 403 error", BOOMLINGS + "getGJLevels21.php");
                } else {
                    geode::log::error("Failed to get daily level info (status code: {})", res->code());
                }
                return Err("Failed to get daily level info (status code: {})", res->code());
            }
            std::string text = res->string().unwrapOr("100");

            FLAlertLayer::create(
                "Daily level ID",
                "The daily level id SHOULD BE " + text,
                "Ok ig"
            )->show();
        });
        
        return Ok(1);
    }
}