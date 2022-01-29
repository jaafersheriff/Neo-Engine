#pragma once

#include "ECS/Component/Component.hpp"
#include "ECS/GameObject.hpp"

using namespace neo;

class SunComponent : public Component {

public:
    SunComponent(GameObject *go) :
        Component(go)
    {}
};
