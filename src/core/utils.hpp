#pragma once

#include <Geode/Geode.hpp>
#include <string>
using namespace geode;

namespace CMentions::utils {
    const std::string BOOMLINGS = "http://www.boomlings.com/database/";

    Result<int> getDailyLevel();
}