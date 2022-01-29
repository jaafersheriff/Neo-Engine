#pragma once

#include "ECS/Component/Component.hpp"

namespace neo {
    class Mesh;

    class MeshComponent : public Component {
    public:
        const Mesh& mMesh;
        MeshComponent(GameObject *go, const Mesh& mesh) :
            Component(go),
            mMesh(mesh)
        {}

    };
}