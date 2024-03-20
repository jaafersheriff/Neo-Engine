#pragma once

#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"
#include "ECS/Component/RenderingComponent/MaterialComponent.hpp"

#include "Loader.hpp"
#include "ResourceManager/MeshResourceManager.hpp"

#include <string>
#include <vector>

namespace neo {
	namespace GLTFImporter {

		struct Node {
			enum class AlphaMode {
				Opaque,
				AlphaTest,
				Transparent
			};

			std::string mName = "";

			MeshHandle mMeshHandle;
			glm::vec3 mMin = glm::vec3(0.f);
			glm::vec3 mMax = glm::vec3(0.f);

			SpatialComponent mSpatial = {};

			AlphaMode mAlphaMode = AlphaMode::Opaque;
			MaterialComponent mMaterial = {};
		};

		struct Scene {
			std::vector<Node> mMeshNodes;
		};

		Scene loadScene(const std::string& fileName, glm::mat4 baseTransform, ResourceManagers& resourceManagers);
	}
}
