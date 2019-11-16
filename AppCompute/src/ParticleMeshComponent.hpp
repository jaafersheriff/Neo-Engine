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
        mMesh->mPrimitiveType = GL_POINTS;
        mMesh->addVertexBuffer(VertexType::Position, 0, 4); // positions
        mMesh->addVertexBuffer(VertexType::Color0, 1, 4); // velocity
        updateBuffers();
    }

    virtual void imGuiEditor() override {
        if (ImGui::DragInt("#Verts", &mNumVerts, 1, 3, 2048)) {
            updateBuffers();
        }
    }

    void updateBuffers() {
        std::vector<float> buffer;
        buffer.resize(mNumVerts * 4);
        for (int i = 0; i < mNumVerts; i++) {
            buffer[i * 4 + 0] = Util::genRandom(-1.f, 1.f);
            buffer[i * 4 + 1] = Util::genRandom(-1.f, 1.f);
            buffer[i * 4 + 2] = Util::genRandom(-1.f, 1.f);
            buffer[i * 4 + 3] = 1.f;
        }
        mMesh->updateVertexBuffer(VertexType::Position, buffer);
        mMesh->updateVertexBuffer(VertexType::Color0, buffer);
    }
};
