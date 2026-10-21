#pragma once

#include <Geode/Result.hpp>
#include <Geode/utils/StringMap.hpp>

#include <string>
#include <vector>

enum class LevelType {
    Daily = 21,
    Weekly = 22,
    Event = 23
};

inline const std::string SECRET = "Wmfd2893gb7";
using ServerResponse = geode::Result<std::string, int>;

arc::Future<ServerResponse> requestGDServersCM(
    const std::string& endpoint,
    const std::string& body,
    int timeout = 10
);

std::unordered_map<std::string, std::string> formatResponseCM(const std::string& response, std::string sep);

std::string buildBodyStringCM(std::initializer_list<std::pair<std::string, std::string>> body);

arc::Future<bool> doWeHaveInternetCM(const std::string& url = "http://connectivitycheck.gstatic.com/generate_204");

geode::utils::StringMap<std::string> formatKV(
    const std::string& str,
    geode::utils::StringMap<std::string> map,
    const std::string& sep = ":"
);

std::vector<std::string> getListSetting(const std::string& setting);

void setListSetting(const std::string& setting, const std::vector<std::string> value);

arc::Future<geode::Result<int>> getSpecialID(LevelType type);

arc::Future<> pauseUntilWeHaveInternet();

inline geode::Result<int> stoicm(const std::string& str) {
    return geode::utils::numFromString<int>(str);
}

inline int stoicm(const std::string& str, int def) {
    return geode::utils::numFromString<int>(str).unwrapOr(def);
}
