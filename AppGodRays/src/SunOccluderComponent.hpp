#pragma once

#include "ECS/Component/Component.hpp"
#include "ECS/GameObject.hpp"

using namespace neo;

class SunOccluderComponent : public Component {

public:
    SunOccluderComponent(GameObject *go) :
        Component(go)
    {}
};
