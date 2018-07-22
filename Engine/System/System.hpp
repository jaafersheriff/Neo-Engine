#pragma once

#include <string>

namespace neo {

    class System {

        public:
            virtual std::string name() = 0;
            virtual void init() {};
            virtual void update(float) {};
            bool active = true;

    };
}