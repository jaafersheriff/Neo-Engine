#pragma once

#include "Systems/System.hpp"

using namespace neo;

class RotationSystem : public System {

    public:
        RotationSystem() :
            System("Camera System")
        {}

        virtual void update(const float dt) override;

};