#pragma once

#include "Component/Component.hpp"

#include "GLObjects/Mesh.hpp"

#include "ext/imgui/imgui.h"

using namespace neo;

class ParticleMeshComponent : public Component {

public:
    Mesh* mMesh;
    int mNumVerts = 256;

    ParticleMeshComponent(GameObject* go) :
        Component(go)
    {
        mMesh = Library::createEmptyMesh("Particles");
        mMesh->addVertexBuffer(VertexType::Position, 0, 4); // positions
        mMesh->addVertexBuffer(VertexType::Color0, 1, 4); // velocity
        // mMesh->addElementBuffer();
        updateBuffers();
    }

    virtual void imGuiEditor() override {
        if (ImGui::DragInt("#Verts", &mNumVerts, 1, 3, 2048)) {
            updateBuffers();
        }
        if (ImGui::Button("Points")) {
            mMesh->mPrimitiveType = GL_POINTS;
        }
        ImGui::SameLine();
        if (ImGui::Button("Triangles")) {
            mMesh->mPrimitiveType = GL_TRIANGLES;
        }
        ImGui::SameLine();
        if (ImGui::Button("Triangle Strip")) {
            mMesh->mPrimitiveType = GL_TRIANGLE_STRIP;
        }
    }

    void updateBuffers() {
        mMesh->updateVertexBuffer(VertexType::Position, mNumVerts * 4);
        mMesh->updateVertexBuffer(VertexType::Color0, mNumVerts * 4);

        std::vector<unsigned> indices;
        indices.resize(mNumVerts * 6);
        int index = 0;
        for (size_t i = 0; i < mNumVerts; i++) {
            uint32_t element = uint32_t(i << 2);
            indices[index++] = element;
            indices[index++] = element + 1;
            indices[index++] = element + 2;
            indices[index++] = element;
            indices[index++] = element + 2;
            indices[index++] = element + 3;
        }
        // mMesh->updateElementBuffer(indices);
    }
};
