#pragma once

#include "ECS/Component/Component.hpp"

using namespace neo;

class MetaballsMeshComponent : public Component {

public:
    Mesh* mMesh;
    MetaballsMeshComponent(GameObject* go) :
        Component(go) {
        mMesh = new Mesh;
    }
};
