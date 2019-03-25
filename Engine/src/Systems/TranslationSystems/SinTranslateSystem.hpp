#pragma once

#include "Systems/System.hpp"

using namespace neo;

class SinTranslateSystem : public System {

    public:
        SinTranslateSystem() :
            System("Camera System")
        {}

        virtual void update(const float dt) override;

};