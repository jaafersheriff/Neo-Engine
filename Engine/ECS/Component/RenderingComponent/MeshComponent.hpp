#pragma once

#include "ECS/Component/Component.hpp"

namespace neo {
    class Mesh;

    struct MeshComponent : public Component {
        Mesh* mMesh;
        MeshComponent(Mesh* mesh)
            : mMesh(mesh)
        {}

        virtual std::string getName() const override {
            return "MeshComponent";
        }
    };
}