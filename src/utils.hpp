#pragma once

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

void notifyError(const std::string& msg);

std::vector<std::string> getListSetting(const std::string& setting);

arc::Future<geode::Result<int>> getSpecialID(LevelType type);