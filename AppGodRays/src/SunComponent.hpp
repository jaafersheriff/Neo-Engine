#pragma once

#include "Component/Component.hpp"
#include "GameObject/GameObject.hpp"

using namespace neo;

class SunComponent : public Component {

public:
    SunComponent(GameObject *go) :
        Component(go)
    {}
};
