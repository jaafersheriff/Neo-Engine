#pragma once

#include "ECS/Component/Component.hpp"

namespace neo
{
    class ShadowCameraComponent : public Component {
    public:
        ShadowCameraComponent(GameObject *go) :
            Component(go)
        {}
    };
}