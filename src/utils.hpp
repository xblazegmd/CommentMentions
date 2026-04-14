#pragma once

#include <Geode/utils/StringMap.hpp>
#include <string>

geode::utils::StringMap<std::string> formatKV(
    const std::string& str,
    geode::utils::StringMap<std::string> map,
    const std::string& sep = ":"
);

void notifyError(const std::string& msg);

arc::Future<bool> doWeTrulyHaveInternet();