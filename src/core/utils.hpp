#pragma once

#include <Geode/Geode.hpp>
#include <Geode/loader/Event.hpp>
#include <Geode/utils/web.hpp>
#include <string>

using namespace geode;
using namespace geode::utils;

namespace CMutils {
    const std::string BOOMLINGS = "http://www.boomlings.com/database/";
    const std::string SECRET = "Wmfd2893gb7";

    void notify(std::string title, std::string msg);

    enum class LevelFetchTarget {
        Daily,
        Weekly
    };

    class LevelFetch {
        public:
            LevelFetch(LevelFetchTarget target);

            int fetchID();
        private:
            std::shared_ptr<EventListener<web::WebTask>> m_reqListener;
            LevelFetchTarget m_target;
    };
}