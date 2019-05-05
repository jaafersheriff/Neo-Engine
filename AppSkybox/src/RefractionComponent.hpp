#pragma once

#include "Component/Component.hpp"

using namespace neo;

class RefractionComponent : public Component {

    public:

        RefractionComponent(GameObject *go, float r = 0.5f) :
            Component(go),
            ratio(r)
        {}

        float ratio = 0.f;
};