#pragma once

#include "Component/Component.hpp"
#include "GameObject/GameObject.hpp"

using namespace neo;

class SelectableComponent : public Component {
    public:
        SelectableComponent(GameObject *go) :
            Component(go)
        {}
};