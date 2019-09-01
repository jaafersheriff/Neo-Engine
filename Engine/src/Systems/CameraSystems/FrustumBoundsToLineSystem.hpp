#pragma once

#include "Systems/System.hpp"

#include "Component/CameraComponent/CameraComponent.hpp"
#include "Component/CameraComponent/FrustumBoundsComponent.hpp"

namespace neo {

    class FrustumBoundsToLineSystem : public System {

        public:
            FrustumBoundsToLineSystem() :
                System("FrustumBoundsToLine System")
            {}

            virtual void update(const float dt) override;
    };
}
