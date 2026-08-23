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

geode::utils::StringMap<std::string> formatKV(
    const std::string& str,
    geode::utils::StringMap<std::string> map,
    const std::string& sep = ":"
);

std::vector<std::string> getListSetting(const std::string& setting);

arc::Future<geode::Result<int>> getSpecialID(LevelType type);

arc::Future<> pauseUntilWeHaveInternet();

inline geode::Result<int> stoicm(const std::string& str) {
    return geode::utils::numFromString<int>(str);
}

inline int stoicm(const std::string& str, int def) {
    return geode::utils::numFromString<int>(str).unwrapOr(def);
}