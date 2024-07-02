#pragma once

#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"
#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/RenderingComponent/MaterialComponent.hpp"

#include "Loader.hpp"
#include "ResourceManager/MeshManager.hpp"

#include <string>
#include <vector>

namespace neo {
	namespace GLTFImporter {

		struct Node {
			std::string mName = "";
			SpatialComponent mSpatial = {};
		};

		struct CameraNode : public Node {
			CameraComponent mCameraComponent;
		};

		struct MeshNode : public Node {
			enum class AlphaMode {
				Opaque,
				AlphaTest,
				Transparent
			};

			MeshHandle mMeshHandle;
			glm::vec3 mMin = glm::vec3(0.f);
			glm::vec3 mMax = glm::vec3(0.f);

			AlphaMode mAlphaMode = AlphaMode::Opaque;
			MaterialComponent mMaterial = {};
		};

		struct Scene {
			std::optional<CameraNode> mCamera = std::nullopt;
			std::vector<MeshNode> mMeshNodes = {};
		};

		Scene loadScene(const std::string& fileName, glm::mat4 baseTransform, ResourceManagers& resourceManagers);
	}
}
