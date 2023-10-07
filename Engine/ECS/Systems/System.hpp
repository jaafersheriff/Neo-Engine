#pragma once

namespace neo {

    class ECS;

    class System {

        public:
            System(const std::string & name) :
                mName(name)
            {}
            ~System() = default;
            System(const System&) = delete;
            System& operator=(const System&) = delete;


            virtual void init(ECS&) {};
            virtual void update(ECS&) {};
            virtual void imguiEditor(ECS&) {};
            bool mActive = true;
            const std::string mName = 0;
    };
}