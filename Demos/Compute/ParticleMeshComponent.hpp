#pragma once

#include "ECS/Component/Component.hpp"

#include "Renderer/GLObjects/Mesh.hpp"
#include "Renderer/Renderer.hpp"

#include "ResourceManager/MeshResourceManager.hpp"

#include "Util/Profiler.hpp"
#include "Util/ServiceLocator.hpp"

#include <imgui.h>
#include <tracy/TracyOpenGL.hpp>

using namespace neo;

namespace Compute {
	class ParticleMeshComponent : public Component {

	public:
		MeshHandle mMeshHandle;
		int mNumParticles = 98304;
		float timeScale = 100.f;
		bool isDirty = true;

		ParticleMeshComponent(MeshResourceManager& meshManager) {
			MeshLoadDetails builder;
			builder.mPrimtive = types::mesh::Primitive::Points;
			builder.mVertexBuffers[types::mesh::VertexType::Position] = {
				4,
				0,
				types::ByteFormats::Float,
				false,
				0,
				0,
				0,
				nullptr
			};

			mMeshHandle = meshManager.asyncLoad("Particles", builder);
			isDirty = true;
		}

		virtual void imGuiEditor() override {
			isDirty = ImGui::DragInt("#Verts", &mNumParticles, 1.f, ServiceLocator<Renderer>::ref().mDetails.mMaxComputeWorkGroupSize.x, 1572864);
			isDirty = ImGui::Button("Reset");
			ImGui::SliderFloat("Time scale", &timeScale, 0.f, 1000.f);
		}

		virtual std::string getName() const override {
			return "ParticleMeshComponent";
		}
	};
}
