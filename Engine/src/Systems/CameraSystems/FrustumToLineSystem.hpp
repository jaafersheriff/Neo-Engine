#pragma once

#include "Systems/System.hpp"

#include "Component/CameraComponent/CameraComponent.hpp"
#include "Component/CameraComponent/FrustumComponent.hpp"

namespace neo {

    class FrustumToLineSystem : public System {

        public:
            FrustumToLineSystem() :
                System("FrustumToLine System")
            {}

            virtual void update(const float dt) override;
    };
}
