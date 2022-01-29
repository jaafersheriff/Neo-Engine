#pragma once

#include "ECS/Component/Component.hpp"

namespace neo {

    class MainCameraComponent : public Component {
    public:
        MainCameraComponent(GameObject *go);
    };
}