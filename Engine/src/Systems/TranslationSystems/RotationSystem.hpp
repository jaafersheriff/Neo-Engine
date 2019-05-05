#pragma once

#include "Systems/System.hpp"

namespace neo {

    class RotationSystem : public System {

    public:
        RotationSystem() :
            System("Rotation System")
        {}

        virtual void update(const float dt) override;

    };
}