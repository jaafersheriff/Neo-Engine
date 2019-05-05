#pragma once

#include "Component/Component.hpp"

class ReflectionComponent : public neo::Component {

    public:

        ReflectionComponent(neo::GameObject *go) :
            neo::Component(go)
        {}
};