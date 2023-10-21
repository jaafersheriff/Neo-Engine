#pragma once

#include "ECS/Component/Component.hpp"

#include "Renderer/GLObjects/Mesh.hpp"
#include "Renderer/Renderer.hpp"

#include "Loader/Library.hpp"

#include "Util/Profiler.hpp"
#include "Util/ServiceLocator.hpp"

#include <imgui.h>
#include <tracy/TracyOpenGL.hpp>

using namespace neo;

namespace Compute {
    class ParticleMeshComponent : public Component {

    public:
        Mesh* mMesh;
        int mNumParticles = 98304;

        ParticleMeshComponent() {
            MeshData meshData;
            mMesh = meshData.mMesh = new Mesh;
            mMesh->mPrimitiveType = GL_POINTS;
            mMesh->addVertexBuffer(VertexType::Position, 0, 4); // positions
            updateBuffers();
            Library::insertMesh("Particles", meshData);
        }

        virtual void imGuiEditor() override {
            if (ImGui::DragInt("#Verts", &mNumParticles, 1.f, ServiceLocator<Renderer>::ref().mDetails.mMaxComputeWorkGroupSize.x, 1572864)) {
                updateBuffers();
            }
            if (ImGui::Button("Reset")) {
                updateBuffers();
            }
        }

        void updateBuffers() {
            TRACY_GPUN("ParticleMeshComponent::updateBuffers");
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

        virtual std::string getName() const override {
            return "ParticleMeshComponent";
        }
    };
}
