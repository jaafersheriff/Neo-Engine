#pragma once

#include <string>

namespace neo {

    class System {

        public:
            System(const std::string & name) :
                name(name)
            {}

            virtual void init() {};
            virtual void update(float) {};
            bool active = true;
            const std::string name = 0;
    };
}