#pragma once

#include "ECS/Component/Component.hpp"

#include "ResourceManager/MeshManager.hpp"

namespace neo {
	struct MeshComponent : public Component {
		MeshHandle mMeshHandle;
		MeshComponent(MeshHandle mesh)
			: mMeshHandle(mesh)
		{}

		virtual std::string getName() const override {
			return "MeshComponent";
		}
	};
}