#pragma once

#include "Component/Component.hpp"


class RefractionComponent : public neo::Component {

    public:

        RefractionComponent(neo::GameObject *go, float r = 0.5f) :
            neo::Component(go),
            ratio(r)
        {}

        float ratio = 0.f;
};