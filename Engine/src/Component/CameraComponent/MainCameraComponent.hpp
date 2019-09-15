#pragma once

#include "Component/Component.hpp"

namespace neo {

    class MainCameraComponent : public Component {
    public:
        MainCameraComponent(GameObject *go) :
            Component(go)
        {}
    };

}