#pragma once

#include <Geode/Geode.hpp>
#include <Geode/Result.hpp>
#include <Geode/loader/Event.hpp>
#include <Geode/utils/web.hpp>
#include <functional>
#include <string>

using namespace geode;
using namespace geode::utils;

namespace CMUtils {
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

            void fetchID(std::function<void(Result<int>)> callback);
        private:
            std::shared_ptr<EventListener<web::WebTask>> m_reqListener;
            LevelFetchTarget m_target;

            int parseResponse(std::string res);
    };
}