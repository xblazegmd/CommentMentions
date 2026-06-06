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
    auto kv = xblazeapi::formatResponse(str, sep);

    utils::StringMap<std::string> ret;
    for (const auto& [k, v] : kv) {
        std::string nk = map.contains(k) ? map[k] : k;
        ret[nk] = v;
    }
    return ret;
}

void notifyError(const std::string& msg) {
    if (!Mod::get()->getSettingValue<bool>("show-errors")) return;
    if (PlayLayer::get() && !Mod::get()->getSettingValue<bool>("show-errors-while-playing")) return;
    xblazeapi::quickErrorNotificationTS(msg);
}

std::vector<std::string> getListSetting(const std::string& setting) {
    auto value = Mod::get()->getSettingValue<std::string>(setting);
    auto split = string::split(value, ",");

    std::vector<std::string> ret;
    for (const auto& item : split) {
        ret.push_back(string::trim(item));
    }
    return ret;
}

arc::Future<Result<int>> getSpecialID(LevelType type) {
    auto res = co_await xblazeapi::requestGDServers("getGJLevels21.php", xblazeapi::buildBodyString({
        { "type", utils::numToString(static_cast<int>(type)) },
        { "secret", xblazeapi::SECRET }
    }));
    if (res.isErr()) {
        log::error("{}", res.unwrapErr());
        co_return Err("{}", res.unwrapErr());
    }

    auto daily = string::split(string::split(res.unwrap(), "#")[0], "|")[0];
    auto dailyID = formatKV(daily, {{"1", "daily"}})["daily"];
    auto intDailyID = utils::numFromString<int>(dailyID);

    if (intDailyID.isErr()) {
        co_return Err(intDailyID.unwrapErr());
    }

    co_return Ok(intDailyID.unwrap());
}

arc::Future<> pauseUntilWeHaveInternet() {
    while (true) {
        if (co_await xblazeapi::doWeHaveInternet()) break;
        co_await xblazeapi::sleepSecs(3);
    }
}