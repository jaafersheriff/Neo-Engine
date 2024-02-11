#pragma once

#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"
#include "ECS/Component/RenderingComponent/MaterialComponent.hpp"

#include "Loader.hpp"

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
			MeshData mMesh;

			SpatialComponent mSpatial = {};

			AlphaMode mAlphaMode = AlphaMode::Opaque;
			MaterialComponent mMaterial = {};
		};

		struct Scene {
			std::vector<Node> mMeshNodes;
		};


		Scene loadScene(const std::string& fileName);
	}
}
