#pragma once

#include <Geode/utils/StringMap.hpp>
#include <string>

const std::string BOOMLINGS = "http://www.boomlings.com/database/";
const std::string SECRET = "Wmfd2893gb7";

geode::utils::StringMap<std::string> formatKV(
    const std::string& str,
    geode::utils::StringMap<std::string> map,
    const std::string& sep = ":"
);