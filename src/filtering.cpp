#include "filtering.hpp"

#include <utils.hpp>

#include <Geode/Geode.hpp>
#include <Geode/loader/Mod.hpp>
#include <Geode/utils/StringMap.hpp>
#include <Geode/utils/string.hpp>

#include <regex>
#include <string>
#include <vector>

using namespace geode::prelude;

std::regex mkRegex(const std::string& text) {
    // Geode's split doesn't work here for my use case
    std::vector<std::string> split;
    split.reserve(text.size());
    for (const char& c : text) {
        split.push_back(std::string(1, c));
    }

    return std::regex(fmt::format("\\b(?:{})\\b", string::join(split, "\\s*")), std::regex::icase | std::regex::optimize);
}

// INAPPROPRIATE WORDS IN 5...
// INAPPROPRIATE WORDS IN 4...
// INAPPROPRIATE WORDS IN 3...
// INAPPROPRIATE WORDS IN 2...
// INAPPROPRIATE WORDS IN 1...
const std::vector<std::regex> blacklistedWords = {
    mkRegex("ass"),
    mkRegex("anus"),
    mkRegex("anal"),
    mkRegex("boob"),
    mkRegex("boobs"),
    mkRegex("bitch"),
    mkRegex("bitches"),
    mkRegex("cock"),
    mkRegex("cum"),
    mkRegex("dick"),
    mkRegex("ejaculation"),
    mkRegex("erect"),
    mkRegex("erection"),
    mkRegex("erpwithme"),
    mkRegex("fag"),
    mkRegex("faggot"),
    mkRegex("fuck"),
    mkRegex("fuk"),
    mkRegex("fuckwithme"),
    mkRegex("fuk"),
    mkRegex("fukwithme"),
    mkRegex("fukmeup"),
    mkRegex("fack"),
    mkRegex("fackwithme"),
    mkRegex("fackmeup"),
    mkRegex("fakwithme"),
    mkRegex("fakmeup"),
    mkRegex("heigui"),
    mkRegex("jiggaboo"),
    mkRegex("jiggerboo"),
    mkRegex("kys"),
    mkRegex("killyourself"),
    mkRegex("masturbate"),
    mkRegex("masturbation"),
    mkRegex("nig"),
    mkRegex("nigg"),
    mkRegex("nigr"),
    mkRegex("niger"), // rip niger such a wonderful country
    mkRegex("nigga"),
    mkRegex("nagga"),
    mkRegex("niggah"),
    mkRegex("niggas"),
    mkRegex("niggga"),
    mkRegex("nigger"),
    mkRegex("nagger"),
    mkRegex("niggger"),
    mkRegex("niggers"),
    mkRegex("niglet"),
    mkRegex("orgasm"),
    mkRegex("retard"),
    mkRegex("retarded"),
    mkRegex("screwmeup"),
    mkRegex("sex"),
    mkRegex("sexual"),
    mkRegex("sexting"),
    mkRegex("sevcum"), // iykyk
    mkRegex("suckmyass"),
    mkRegex("suckmydick"),
    mkRegex("suckme"),
    mkRegex("suckdick"),
    mkRegex("suckmytit"),
    mkRegex("touchme"),
    mkRegex("touchingme"),
    mkRegex("unfuckable"),
    mkRegex("wigga"),
    mkRegex("wigger")
};

static std::vector<std::regex> blacklist;

$execute {
    for (const auto& word : blacklistedWords) {
        blacklist.push_back(word);
    }

    auto list = getListSetting("word-blacklist");
    for (const auto& item : list) {
        blacklist.push_back(mkRegex(item));
    }
}

const utils::StringMap<std::string> replacementMap = {
    {"0", "o"},
    {"1", "i"},
    {"2", "s"},
    {"3", "e"},
    {"4", "a"},
    {"5", "s"},
    {"6", "g"},
    {"7", "t"},
    {"8", "b"},
    {"9", "p"},
    {"10", "io"},
    {"11", "h"},
    {"12", "is"},
    {"@", "a"},
    {"!", "i"}
};

std::string normalizeComment(const std::string& comment) {
    std::string res = comment;
    string::toLowerIP(res);
    for (const auto& [k, v] : replacementMap) {
        string::replaceIP(res, k, v);
    }
    return res;
}

bool isWhitelisted(const std::string& word) {
    auto w = string::replace(word, " ", ""); // Normalize
    auto whitelist = getListSetting("word-whitelist");
    for (const auto& whitelistedWord : whitelist) {
        if (w == whitelistedWord) {
            log::info("Found innapropriate word '{}', however, it'll be ignored as it is whitelisted", w);
            return true;
        }
    }
    return false;
}

bool isInappropriate(const std::string& comment) {
    auto normalized = normalizeComment(comment);
    for (auto& r : blacklist) {
        std::smatch match;
        if (std::regex_search(normalized, match, r)) {
            if (isWhitelisted(match.str())) continue;
            return true;
        }
    }
    return false;
}