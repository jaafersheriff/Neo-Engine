#pragma once

#include "ECS/Component/Component.hpp"

using namespace neo;

class DecalRenderable: public Component {
public:
     DecalRenderable(GameObject *go) :
        Component(go)
    {}

};