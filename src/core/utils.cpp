#include "utils.hpp"

#include <Geode/Geode.hpp>
#include <Geode/utils/string.hpp>
#include <string>

using namespace geode::prelude;
using namespace geode::utils;

namespace CMUtils {
    bool contains(std::string const& text, std::string const& contains) {
        auto pieces = string::split(text, " ");

        for (const auto& piece : pieces) {
            auto cleanPiece = cleanNameString(piece);
            if (string::toLower(cleanPiece) == contains) {
                return true;
            }
        }
        return false;
    }

    std::string cleanNameString(std::string name) {
        // Proper possessive handling
        const auto len = name.size();
        if (len > 2 && string::endsWith(name, "'s")) {
            name.erase(len - 2);
        } else if (len > 1 && string::endsWith(name, "s'")) {
            name.erase(len - 1);
        }

        return cleanString(name);
    }

    std::string cleanString(std::string const& text) {
        std::string res;
        res.reserve(text.size());

        for (unsigned char c : text) {
            if (isAlnum(c)) {
                res.push_back(c);
            }
        }
        return res;
    }

    bool isAlnum(char c) {
        return (c >= 'a' && c <= 'z') ||
               (c >= 'A' && c <= 'Z') ||
               (c >= '0' && c <= '9');
    }
}