#pragma once

#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"
#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/RenderingComponent/MaterialComponent.hpp"

#include "ResourceManager/MeshManager.hpp"

#include <string>
#include <vector>
#include <functional>

namespace neo {
	class ECS;

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
			bool mDoubleSided = false;
		};

		using MeshNodeOp = std::function<void(ECS&, const MeshNode&)>;
		using CameraNodeOp = std::function<void(ECS&, const CameraNode&)>;
		void loadScene(std::string fileName, glm::mat4 baseTransform, ResourceManagers& resourceManagers, 
			ECS& ecs, MeshNodeOp meshOperator, CameraNodeOp cameraOperator);
	}
}
