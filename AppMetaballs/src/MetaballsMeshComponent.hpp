#pragma once

#include "ECS/Component/Component.hpp"

using namespace neo;

class MetaballsMeshComponent : public Component {

public:
    MetaballsMeshComponent(GameObject* go) :
        Component(go) {
    }
};
