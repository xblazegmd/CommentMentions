#pragma once

#include <Geode/utils/StringMap.hpp>
#include <string>
#include <vector>

geode::utils::StringMap<std::string> formatKV(
    const std::string& str,
    geode::utils::StringMap<std::string> map,
    const std::string& sep = ":"
);

void notifyError(const std::string& msg);

std::vector<std::string> getListSetting(const std::string& setting);