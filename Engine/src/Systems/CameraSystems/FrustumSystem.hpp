#pragma once

#include "Systems/System.hpp"

#include "Component/CameraComponent/CameraComponent.hpp"

namespace neo {

    class FrustumSystem : public System {

    public:
        FrustumSystem() :
            System("Frustum System")
        {}

        virtual void update(const float dt) override;
    };
}
