#pragma once

#include "ECS/Systems/System.hpp"

#include "ECS/Component/CameraComponent/CameraComponent.hpp"

namespace neo {

    class FrustumSystem : public System {

    public:
        FrustumSystem() :
            System("Frustum System")
        {}

        virtual void update(const float dt) override;
    };
}
