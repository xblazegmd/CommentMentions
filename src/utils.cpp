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

std::vector<std::string> getListSetting(const std::string& setting) {
    auto value = Mod::get()->getSettingValue<std::string>(setting);
    auto split = string::split(value, ",");

    std::vector<std::string> ret;
    for (const auto& item : split) {
        ret.push_back(string::trim(item));
    }
    return ret;
}