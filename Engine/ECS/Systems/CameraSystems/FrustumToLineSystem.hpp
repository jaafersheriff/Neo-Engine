#pragma once

#include "ECS/Systems/System.hpp"

namespace neo {

    class FrustumToLineSystem : public System {

        public:
            FrustumToLineSystem() :
                System("FrustumToLine System")
            {}

            virtual void update(ECS& ecs) override;
    };
}
