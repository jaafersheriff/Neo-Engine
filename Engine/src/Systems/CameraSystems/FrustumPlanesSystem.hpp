#pragma once

#include "Systems/System.hpp"

#include "Component/CameraComponent/CameraComponent.hpp"

namespace neo {

    class FrustumPlanesSystem : public System {

    public:
        FrustumPlanesSystem() :
            System("FrustumPlanes System")
        {}

        virtual void update(const float dt) override;
    };
}
