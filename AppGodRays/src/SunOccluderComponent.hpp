#pragma once

#include "Component/Component.hpp"
#include "GameObject/GameObject.hpp"

using namespace neo;

class SunOccluderComponent : public Component {

public:
    SunOccluderComponent(GameObject *go) :
        Component(go)
    {}
};
