#include "utils.hpp"

#include <Geode/Geode.hpp>
#include <string>

using namespace geode::prelude;

utils::StringMap<std::string> formatKV(
    const std::string& str,
    geode::utils::StringMap<std::string> map,
    const std::string& sep
) {
    auto parts = string::split(str, sep);

    utils::StringMap<std::string> kv;
    for (int i = 0; i + 1 < parts.size(); i += 2) {
        kv[parts[i]] = parts[i + 1];
    }

    utils::StringMap<std::string> ret;
    for (const auto& [k, v] : kv) {
        auto it = map.find(k);
        std::string newk = it != map.end() ? it->second : k;
        ret[newk] = v;
    }

    return ret;
}