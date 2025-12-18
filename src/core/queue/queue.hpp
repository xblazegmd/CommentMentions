/// Copy-paste of history.hpp but with some changes LOL
#pragma once

#include <core/history/history.hpp>
#include <string>
#include <unordered_map>

namespace queue {
    void writeToQueue(history::History contents);
    history::History loadQueue();
    void addToQueue(std::unordered_map<std::string, std::string> contents);
    void clearQueue();
}