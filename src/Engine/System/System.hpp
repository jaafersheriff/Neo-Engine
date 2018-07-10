#pragma once

namespace neo {
    class System {
        public:
            virtual void init() = 0;
            virtual void update(float) = 0;
    };
}