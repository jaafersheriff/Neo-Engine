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
		float timeScale = 100.f;

		ParticleMeshComponent() {
			mMesh = new Mesh;
			mMesh->mPrimitiveType = types::mesh::Primitive::Points;
			mMesh->addVertexBuffer(
				types::mesh::VertexType::Position,
				4,
				0,
				types::ByteFormats::Float,
				false,
				0,
				0,
				0,
				nullptr
			);

			updateBuffers();
			Library::insertMesh("Particles", mMesh);
		}

		virtual void imGuiEditor() override {
			if (ImGui::DragInt("#Verts", &mNumParticles, 1.f, ServiceLocator<Renderer>::ref().mDetails.mMaxComputeWorkGroupSize.x, 1572864)) {
				updateBuffers();
			}
			if (ImGui::Button("Reset")) {
				updateBuffers();
			}
			ImGui::SliderFloat("Time scale", &timeScale, 0.f, 1000.f);
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
				positions[i * 4 + 3] = 1.f; // TODO - this is useless and costs perf. Get rid of it
			}
			mMesh->updateVertexBuffer(
				types::mesh::VertexType::Position, 
				static_cast<uint32_t>(positions.size()),
				static_cast<uint32_t>(positions.size() * sizeof(float)),
				reinterpret_cast<uint8_t*>(positions.data())
			);
		}

		virtual std::string getName() const override {
			return "ParticleMeshComponent";
		}
	};
}
