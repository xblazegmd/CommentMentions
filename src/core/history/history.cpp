#include "history.hpp"

#include <Geode/Geode.hpp>

using namespace geode::prelude;

namespace history {
    void writeHistory(History contents) {
        Mod::get()->setSavedValue<History>("history", contents);
    }

    History loadHistory() {
        return Mod::get()->getSavedValue<History>("history");
    }

    void updateHistory(History contents) {
        auto history = loadHistory();
        
        // In here we'll handle the "mention-history-maxsize" configuration option
        auto maxsize = Mod::get()->getSettingValue<int64_t>("mention-history-maxsize");
        if (history.size() > maxsize) {
            history.erase(history.begin());
        }

        for (const auto& item : contents) {
            history.push_back(item);
        }
    }

    bool mentionExists(std::unordered_map<std::string, std::string> mention) {
        auto hist = loadHistory();
        for (const auto& item : hist) {
            // Differ between mention with the messageID
            auto msgID = item.find("messageID");
            if (msgID == item.end()) {
                log::error("Could not find 'messageID' in: {}", item);
                continue;
            };

            if (msgID->second == mention["messageID"]) return true;
        }
        return false;
    }
}