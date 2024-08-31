#pragma once

#include "ECS/Component/Component.hpp"

#include "ResourceManager/MeshManager.hpp"

namespace neo {
	START_COMPONENT(ImGuiDrawComponent);
		ImGuiDrawComponent() {}
		MeshHandle mMeshHandle;

	END_COMPONENT();
}