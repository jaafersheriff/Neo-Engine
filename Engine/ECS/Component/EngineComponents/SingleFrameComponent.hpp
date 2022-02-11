#pragma once

#include "ECS/Component/Component.hpp"
#include "ECS/GameObject.hpp"

namespace neo {

    class SingleFrameComponent : public Component {

    public:
        SingleFrameComponent(GameObject *go)
            : Component(go)
        {}
    };
}