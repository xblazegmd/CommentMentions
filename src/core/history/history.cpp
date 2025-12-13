#include "history.hpp"
#include "Geode/utils/file.hpp"

#include <Geode/Geode.hpp>
#include <Geode/utils/coro.hpp>
#include <filesystem>
#include <string>
#include <vector>

using namespace geode::prelude;
using namespace geode::utils;

namespace history {
    std::filesystem::path getHistoryPath() {
        auto saveDir = Mod::get()->getSaveDir();
        return saveDir / "history.json";
    }

    Result<> writeHistory(std::vector<std::string> contents) {
        auto history = getHistoryPath();
        History data;
        data.history = contents;

        return file::writeToJson(history, data);
    }

    Result<History> loadHistory() {
        auto history = getHistoryPath();
        return file::readFromJson<History>(history);
    }

    Result<> updateHistory(std::vector<std::string> contents) {
        auto historyPath = getHistoryPath();
        auto history = co_await loadHistory(); // co_await in this case is like ? on rust
        
        // In here we'll handle the "mention-history-maxsize" configuration option
        auto maxsize = Mod::get()->getSettingValue<int64_t>("mention-history-maxsize");
        if (history.history.size() > maxsize) {
            history.history.erase(history.history.begin());
        }

        for (const auto& item : contents) {
            history.history.push_back(item);
        }
        co_return writeHistory(history.history);
    }
}