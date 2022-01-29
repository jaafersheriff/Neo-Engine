#pragma once

#include "ECS/Systems/System.hpp"


namespace neo {

    class FrustaFittingSystem : public System {

    public:

        FrustaFittingSystem() :
            System("FrustaFitting System") {
        }

        virtual void update(const float dt) override;
    };
}
