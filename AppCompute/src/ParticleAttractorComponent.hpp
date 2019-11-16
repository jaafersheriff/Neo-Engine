#pragma once

#include "Component/Component.hpp"

using namespace neo;

class ParticleAttractorComponent : public Component {

public:

    ParticleAttractorComponent(GameObject* go) :
        Component(go)
    {
    }
};
