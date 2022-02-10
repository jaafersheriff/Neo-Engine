#pragma once

#include "ECS/Component/Component.hpp"

#include <inttypes.h>

namespace neo {

    class SelectableComponent : public Component {
    public:

        SelectableComponent(GameObject* go) :
            Component(go),
            mID(sCounter++)
        {}

        uint32_t mID = 0;

    private:
        static uint32_t sCounter;
    };
}