#pragma once

#include <Geode/Geode.hpp>
#include <string>

using namespace geode;

namespace CMentions::utils {
    const std::string BOOMLINGS = "http://www.boomlings.com/database/";
    const std::string SECRET = "Wmfd2893gb7";

    void notify(std::string title, std::string msg);
}