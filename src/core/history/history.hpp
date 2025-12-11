#pragma once

#include <Geode/Geode.hpp>
#include <filesystem>
#include <matjson.hpp>
#include <matjson/std.hpp>
#include <string>
#include <vector>

using namespace geode::prelude;

namespace history {
    struct History {
        std::vector<std::string> history;
    };


    std::filesystem::path getHistoryPath();

    Result<> writeHistory(std::vector<std::string> contents);
    Result<History> loadHistory();
    Result<> updateHistory(std::vector<std::string> contents);
}

template<>
struct matjson::Serialize<history::History> {
    static Result<history::History> fromJson(const matjson::Value& value) {
        GEODE_UNWRAP_INTO(std::vector<std::string> history, value["history"].as<std::vector<std::string>>());
        return Ok(history::History { history });
    }

    static matjson::Value toJson(const history::History& history) {
        return matjson::makeObject({
            { "history", history.history }
        });
    }
};