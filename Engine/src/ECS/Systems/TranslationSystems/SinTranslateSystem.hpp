#pragma once

#include "ECS/Systems/System.hpp"

namespace neo {

    class SinTranslateSystem : public System {

    public:
        SinTranslateSystem() :
            System("SinTranslate System")
        {}

        virtual void update(const float dt) override;

    };
}