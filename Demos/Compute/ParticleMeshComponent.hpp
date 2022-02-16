#pragma once

#include "ECS/Component/Component.hpp"

#include "Renderer/GLObjects/Mesh.hpp"

#include "ext/imgui/imgui.h"

using namespace neo;

class ParticleMeshComponent : public Component {

public:
    Mesh* mMesh;
    int mNumParticles = 98304;

    ParticleMeshComponent(GameObject* go) :
        Component(go)
    {
        mMesh = Library::createEmptyMesh("Particles");
        mMesh->mPrimitiveType = GL_POINTS;
        mMesh->addVertexBuffer(VertexType::Position, 0, 4); // positions
        updateBuffers();
    }

    virtual void imGuiEditor() override {
        if (ImGui::DragInt("#Verts", &mNumParticles, 1.f, Renderer::NEO_MAX_COMPUTE_GROUP_SIZE.x, 1572864)) {
            updateBuffers();
        }
        if (ImGui::Button("Reset")) {
            updateBuffers();
        }
    }

    void updateBuffers() {
        std::vector<float> positions;
        positions.resize(mNumParticles * 4);
        for (int i = 0; i < mNumParticles; i++) {
            glm::vec3 pos = glm::normalize(util::genRandomVec3(-1.f, 1.f));
            positions[i * 4 + 0] = pos.x;
            positions[i * 4 + 1] = pos.y;
            positions[i * 4 + 2] = pos.z;
            positions[i * 4 + 3] = 1.f;
        }
        mMesh->updateVertexBuffer(VertexType::Position, positions);
    }
};
