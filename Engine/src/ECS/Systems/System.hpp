#pragma once

#include <string>

namespace neo {

    class System {

        public:
            System(const std::string & name) :
                mName(name)
            {}

            virtual void init() {};
            virtual void update(const float) {};
            virtual void imguiEditor() {};
            bool mActive = true;
            const std::string mName = 0;
    };
}