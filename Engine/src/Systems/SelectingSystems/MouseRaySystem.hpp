#pragma once

#include "Systems/System.hpp"

namespace neo {

    class MouseRaySystem : public System {

    public:
        MouseRaySystem() :
            System("MouseRay System")
        {}


        virtual void update(const float dt) override;
    };
}
