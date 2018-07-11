#pragma once

namespace neo {

    class System {

        public:
            virtual void init() {};
            virtual void update(float) {};
    };
}