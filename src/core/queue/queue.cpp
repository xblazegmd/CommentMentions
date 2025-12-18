/// Copy-paste of history.cpp but with some changes LOL
#include "queue.hpp"

#include <Geode/Geode.hpp>
#include <core/history/history.hpp>
#include <unordered_map>

using namespace geode::prelude;

namespace queue {
    // Still uses the history::History type since it's literally the same type I need anyways
    void writeToQueue(history::History contents) {
        Mod::get()->setSavedValue<history::History>("queue", contents);
    }

    history::History loadQueue() {
        return Mod::get()->getSavedValue<history::History>("queue");
    }

    void addToQueue(std::unordered_map<std::string, std::string> mention) {
        auto q = loadQueue();

        /* Considering I don't thing the queue will get THAT big and that it gets cleared anyways
        I won't add a maximum size thing like the one in the comment history */

        q.push_back(mention);
        writeToQueue(q);
    }

    void clearQueue() {
        history::History emptyLol;
        Mod::get()->setSavedValue<history::History>("queue", emptyLol);
    }
}