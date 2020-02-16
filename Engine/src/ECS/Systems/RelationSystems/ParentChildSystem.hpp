#pragma once

#include "ECS/Systems/System.hpp"

namespace neo {

    class ParentChildSystem : public System {

    public:
        ParentChildSystem() :
            System("ParentChild System")
        {}

        virtual void update(const float dt) override;

    };
}