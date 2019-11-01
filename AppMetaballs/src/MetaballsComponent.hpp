#pragma once

#include "Component/Component.hpp"

using namespace neo;

class MetaballsComponent : public Component {

public:
    MetaballsComponent(GameObject* go) :
        Component(go)
    {}
};
