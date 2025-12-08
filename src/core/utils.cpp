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

        std::string params = "type=21&secret=" + SECRET;

        bool succeded = true;
        std::string errMsg = "";
        web::WebRequest()
            .userAgent("")
            .timeout(std::chrono::seconds(10))
            .bodyString(params)
            .post(BOOMLINGS + "getGJLevels21.php")
            .listen([succeded, errMsg](web::WebResponse *res) mutable {
                if (!res || !res->ok()) {
                    succeded = false;
                    if (res->code() == 403) {
                        geode::log::error("Request to {} returned a 403 error", BOOMLINGS + "getGJLevels21.php");
                        errMsg = "Request to {} returned a 403 error", BOOMLINGS + "getGJLevels21.php";
                    } else {
                        geode::log::error("Failed to get daily level info (status code: {})", res->code());
                        errMsg = "Failed to get daily level info (status code: " + std::to_string(res->code()) + ")";
                    }
                    return;
                }
                geode::log::debug("response's ok");
                std::string text = res->string().unwrapOr("100");
                geode::log::debug(text);

                // FLAlertLayer::create(
                //     "Daily level ID",
                //     "The daily level id SHOULD BE " + text,
                //     "Ok ig"
                // )->show();
            });
        
        if (!succeded) {
            return Err(errMsg);
        }
        
        geode::log::debug("everything went 'fine'");
        return Ok(1);
    }
}