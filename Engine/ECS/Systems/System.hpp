#pragma once

#include <string>

namespace neo {

    class ECS;

    class System {

        public:
            System(const std::string & name) :
                mName(name)
            {}

            virtual void init(ECS&) {};
            virtual void update(ECS&) {};
            virtual void imguiEditor(ECS&) {};
            bool mActive = true;
            const std::string mName = 0;
    };
}