#pragma once

#include "ECS/Systems/System.hpp"

#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/CameraComponent/FrustumComponent.hpp"

namespace neo {

    class FrustumToLineSystem : public System {

        public:
            FrustumToLineSystem() :
                System("FrustumToLine System")
            {}

            virtual void update(const float dt) override;
    };
}
