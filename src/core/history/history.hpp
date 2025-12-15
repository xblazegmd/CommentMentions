#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace history {
    using History = std::vector<std::unordered_map<std::string, std::string>>;

    void writeHistory(History contents);
    History loadHistory();
    void updateHistory(History contents);
}