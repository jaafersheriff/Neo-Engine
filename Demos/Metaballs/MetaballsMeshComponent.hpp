#pragma once

#include "ECS/Component/Component.hpp"

#include "Renderer/GLObjects/Mesh.hpp"

using namespace neo;

namespace Metaballs {
    class MetaballsMeshComponent : public Component {

    public:
        Mesh* mMesh;
        MetaballsMeshComponent(GameObject* go) :
            Component(go) {
            mMesh = new Mesh;
        }
    };
}
