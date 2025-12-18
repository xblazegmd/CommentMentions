#pragma once

#include <Geode/Geode.hpp>
#include <Geode/Result.hpp>
#include <Geode/loader/Event.hpp>
#include <Geode/utils/web.hpp>
#include <string>

using namespace geode;
using namespace geode::utils;

namespace CMUtils {
    const std::string BOOMLINGS = "http://www.boomlings.com/database/";
    const std::string SECRET = "Wmfd2893gb7";

    void notify(std::string title, std::string msg);
}