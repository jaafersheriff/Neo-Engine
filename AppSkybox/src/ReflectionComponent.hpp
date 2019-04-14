#pragma once

#include "Component/Component.hpp"

using namespace neo;
class ReflectionComponent : public Component {

    public:

        ReflectionComponent(GameObject *go) :
            Component(go)
        {}
};