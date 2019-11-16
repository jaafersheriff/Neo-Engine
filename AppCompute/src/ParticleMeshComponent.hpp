#pragma once

#include "Component/Component.hpp"

#include "GLObjects/Mesh.hpp"

#include "ext/imgui/imgui.h"

using namespace neo;

class ParticleMeshComponent : public Component {

public:
    Mesh* mMesh;
    int mNumVerts = 2048;

    ParticleMeshComponent(GameObject* go) :
        Component(go)
    {
        mMesh = Library::createEmptyMesh("Particles");
        mMesh->mPrimitiveType = GL_POINTS;
        mMesh->addVertexBuffer(VertexType::Position, 0, 4); // positions
        mMesh->addVertexBuffer(VertexType::Color0, 1, 4); // velocity
        updateBuffers();
    }

    virtual void imGuiEditor() override {
        if (ImGui::DragInt("#Verts", &mNumVerts, 1, 1, 1<<16)) {
            updateBuffers();
        }
        if (ImGui::Button("Reset")) {
            updateBuffers();
        }
    }

    void updateBuffers() {
        std::vector<float> positions;
        std::vector<float> velocities;
        positions.resize(mNumVerts * 4);
        velocities.resize(mNumVerts * 4);
        for (int i = 0; i < mNumVerts; i++) {
            glm::vec3 pos = glm::normalize(Util::genRandomVec3(-1.f, 1.f));
            positions[i * 4 + 0] = pos.x;
            positions[i * 4 + 1] = pos.y;
            positions[i * 4 + 2] = pos.z;
            positions[i * 4 + 3] = 1.f;

            velocities[i * 4 + 0] = 0.f;
            velocities[i * 4 + 1] = 0.f;
            velocities[i * 4 + 2] = 0.f;
            velocities[i * 4 + 3] = 0.f;
        }
        mMesh->updateVertexBuffer(VertexType::Position, positions);
        mMesh->updateVertexBuffer(VertexType::Color0, velocities);
    }
};
