#pragma once

#include "ECS/Component/Component.hpp"

#include "ResourceManager/MeshManager.hpp"

namespace neo {
	START_COMPONENT(MeshComponent);
		MeshComponent(MeshHandle mesh)
			: mMeshHandle(mesh)
		{}
		MeshHandle mMeshHandle;
	END_COMPONENT();
}