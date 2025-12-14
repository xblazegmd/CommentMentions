#pragma once

#include <Geode/Result.hpp>
#include <Geode/utils/web.hpp>
#include <functional>
#include <string>

using namespace geode;
using namespace geode::utils;

namespace levelFetch {
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