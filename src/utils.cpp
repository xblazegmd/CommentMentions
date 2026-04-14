#include "utils.hpp"

#include <Geode/Geode.hpp>
#include <string>

#include <xblazegmd.geode-api/include/XblazeAPI.hpp>

using namespace geode::prelude;

utils::StringMap<std::string> formatKV(
    const std::string& str,
    geode::utils::StringMap<std::string> map,
    const std::string& sep
) {
    auto kv = xblazeapi::formatResponse(str);

    utils::StringMap<std::string> ret;
    for (const auto& [k, v] : kv) {
        auto it = map.find(k);
        std::string newk = it != map.end() ? it->second : k;
        ret[newk] = v;
    }

    return ret;
}

void notifyError(const std::string& msg) {
    if (!Mod::get()->getSettingValue<bool>("show-errors")) return;
    xblazeapi::quickErrorNotificationTS(msg);
}

arc::Future<bool> doWeTrulyHaveInternet() {
    auto res = co_await web::WebRequest()
        .userAgent("GeometryDash/2.2081 CommentMentions/1.0.0-beta.2")
        .timeout(std::chrono::seconds(10))
        .get("http://connectivitycheck.gstatic.com/generate_204");
    co_return res.ok();
}