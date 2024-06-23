#pragma once

#include "ECS/Component/Component.hpp"

#include "ResourceManager/MeshManager.hpp"

namespace neo {
	START_COMPONENT(MeshComponent);
		MeshHandle mMeshHandle;
		MeshComponent(MeshHandle mesh)
			: mMeshHandle(mesh)
		{}
	END_COMPONENT();
}