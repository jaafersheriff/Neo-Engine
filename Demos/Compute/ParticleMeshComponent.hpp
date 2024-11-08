#pragma once

#include "ECS/Component/Component.hpp"

#include "Renderer/GLObjects/Mesh.hpp"
#include "Renderer/Renderer.hpp"

#include "ResourceManager/MeshManager.hpp"

#include "Util/Profiler.hpp"
#include "Util/ServiceLocator.hpp"

#include <imgui.h>
#include <tracy/TracyOpenGL.hpp>

using namespace neo;

namespace Compute {
	START_COMPONENT(ParticleMeshComponent);
		MeshHandle mMeshHandle;
		int mNumParticles = 98304;
		float timeScale = 100.f;
		bool isDirty = true;

		ParticleMeshComponent(MeshHandle handle)
			: mMeshHandle(handle)
		{
			isDirty = true;
		}

		virtual void imGuiEditor() override {
			isDirty = ImGui::DragInt("#Verts", &mNumParticles, 1.f, ServiceLocator<Renderer>::value().getDetails().mMaxComputeWorkGroupSize.x, 1572864);
			isDirty = ImGui::Button("Reset");
			ImGui::SliderFloat("Time scale", &timeScale, 0.f, 1000.f);
		}

	END_COMPONENT();
}
