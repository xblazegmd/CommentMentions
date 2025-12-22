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

    /**
     * Does "this that" have "that" in it?
     *
     * The logic in this function is made specifically for finding mentions in comments
     */
    bool contains(std::string const& text, std::string const& contains);

    /**
     * Basically `cleanString` but with proper cleanup for "'s" or "s'"
     */
    std::string cleanNameString(std::string name);

    /**
     * Removes all special characters from a string
     */
    std::string cleanString(std::string const& text);

    /**
     * Why not std::isalnum instead? Idk
     */
    bool isAlnum(char c);
}