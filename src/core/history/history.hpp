#pragma once

#include <Geode/Geode.hpp>
#include <Geode/Result.hpp>
#include <filesystem>
#include <matjson.hpp>
#include <matjson/std.hpp>
#include <string>
#include <unordered_map>
#include <vector>

using namespace geode::prelude;

namespace history {
    struct History {
        std::vector<std::unordered_map<std::string, std::string>> history;
    };


    std::filesystem::path getHistoryPath();

    Result<> writeHistory(std::vector<std::unordered_map<std::string, std::string>> contents);
    Result<History> loadHistory();
    Result<> updateHistory(std::vector<std::unordered_map<std::string, std::string>> contents);
}

template<>
struct matjson::Serialize<history::History> {
    static Result<history::History> fromJson(const matjson::Value& value) {
        auto abcd = value["history"].as<std::vector<std::unordered_map<std::string, std::string>>>();
        if (abcd.isErr()) {
            return Err(abcd.unwrapErr());
        }
        auto hist = abcd.unwrap();
        return Ok(history::History{ .history = hist } );
    }

    static matjson::Value toJson(const history::History& history) {
        return matjson::makeObject({
            { "history", history.history }
        });
    }
};