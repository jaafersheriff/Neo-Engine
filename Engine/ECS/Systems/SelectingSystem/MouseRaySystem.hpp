#pragma once

#include "ECS/Systems/System.hpp"

namespace neo {

    class MouseRaySystem : public System {

    public:
        MouseRaySystem(bool show = false) :
            System("MouseRay System"),
            mShowRay(show)
        {}


        virtual void update(const float dt) override;
        virtual void imguiEditor() override;

    private:
        bool mShowRay;
    };
}
