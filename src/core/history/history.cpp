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
}