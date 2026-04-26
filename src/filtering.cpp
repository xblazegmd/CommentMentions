#include "filtering.hpp"

#include <Geode/Result.hpp>
#include <Geode/loader/Mod.hpp>
#include <Geode/utils/StringMap.hpp>
#include <Geode/utils/string.hpp>

#include <cerrno>
#include <cstring>
#include <fstream>
#include <regex>
#include <string>
#include <vector>

using namespace geode::prelude;

// INAPROPRIATE WORDS IN 5...
// INAPROPRIATE WORDS IN 4...
// INAPROPRIATE WORDS IN 3...
// INAPROPRIATE WORDS IN 2...
// INAPROPRIATE WORDS IN 1...
const std::vector<std::string> blacklist = {
    "ass",
    "abbie",
    "abe",
    "abie",
    "abeed",
    "anus",
    "anal",
    "boob",
    "boobs",
    "cock",
    "cracker",
    "cum",
    "dick",
    "ejaculation",
    "erect",
    "erection",
    "fag",
    "faggot",
    "fuck",
    "fuckwithme",
    "fack",
    "heigui",
    "jiggaboo",
    "jiggerboo",
    "kys",
    "killyourself",
    "masturbate",
    "masturbation",
    "nig",
    "nigg",
    "nigr",
    "niger",
    "nigga",
    "niggah",
    "niggas",
    "nigger",
    "niggers",
    "niglet",
    "orgasm",
    "retard",
    "retarded",
    "sex",
    "sexual",
    "sexting",
    "suck",
    "suckmyass",
    "suckmydick",
    "suckme",
    "suckdick",
    "suckmytit",
    "unfuckable",
    "wigga",
    "wigger",
};

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
    {"@", "a"}
};

std::string normalizeComment(const std::string& comment) {
    std::string res = comment;
    string::toLowerIP(res);
    for (const auto& [k, v] : replacementMap) {
        string::replaceIP(res, k, v);
    }
    return res;
}

Result<std::vector<std::string>> loadStrictBlacklist() {
    std::vector<std::string> ret;

    auto path = Mod::get()->getResourcesDir() / "blacklist.txt";
    std::ifstream file(path);
    if (!file.is_open()) {
        return Err("Failed to open blacklist file: {}", std::strerror(errno));
    }

    std::string str;
    while (std::getline(file, str)) {
        if (str.starts_with("#") || str.empty()) continue;
        ret.push_back(str);
    }

    file.close();
    return Ok(std::move(ret));
}

Result<std::regex> strictBlacklistRegex() {
    static auto strictBlacklist = loadStrictBlacklist();
    if (strictBlacklist.isErr()) {
        return Err("{}", strictBlacklist.unwrapErr());
    }
    return Ok(std::regex("\\b(?:" + string::join(strictBlacklist.unwrap(), "|") + ")\\b", std::regex::icase | std::regex::optimize));
}

bool isInapropriateImpl(std::regex regex, const std::string& comment) {
    auto normalized = normalizeComment(comment);
    std::smatch match;
    if (std::regex_search(normalized, match, regex)) {
        return true;
    }
    return false;
}

bool isInapropriate(const std::string& comment) {
    static std::regex r = std::regex("\\b(?:" + string::join(blacklist, "|") + ")\\b", std::regex::icase | std::regex::optimize);
    return isInapropriateImpl(r, comment);
}

bool isInapropriateStrict(const std::string &comment) {
    static auto r = strictBlacklistRegex();
    if (r.isErr()) {
        log::error("Failed to load strict blacklist: {}", r.unwrapErr());
        return false;
    }
    return isInapropriateImpl(r.unwrap(), comment);
}