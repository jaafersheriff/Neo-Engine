#pragma once

#include "Component/Component.hpp"

#include "GLObjects/Mesh.hpp"

#include "ext/imgui/imgui.h"

namespace neo {

    class ComputeMeshComponent : public Component {

    public:
        Mesh* mComputeMesh;
        int mNumVerts = 256;

        ComputeMeshComponent(GameObject* go) :
            Component(go)
        {
            Mesh::MeshBuffers buffers;
            for (int i = 0; i < mNumVerts; i++) {
                buffers.vertices.push_back(0);
            }

            mComputeMesh = new Mesh(buffers);
            mComputeMesh->upload();
        }
    };
}
