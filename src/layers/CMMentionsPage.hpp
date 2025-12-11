#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

namespace layers {
    class CMMentionsPage : public CCLayer {
        protected:
            bool init();
        public:
            static CMMentionsPage* create();
    };
}