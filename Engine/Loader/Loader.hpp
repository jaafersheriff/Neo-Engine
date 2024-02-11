#pragma once

#include "GL/glew.h"

#include <optional>

// DEPRECATED
#include "ECS/Component/RenderingComponent/MaterialComponent.hpp"

namespace neo {

	class Engine;
	class Mesh;
	class Texture;
	struct TextureFormat;
	namespace GLTFImporter {
		struct Scene;
	}

   struct MeshData {
		Mesh* mMesh;
		glm::vec3 mMin{ 0.f, 0.f, 0.f };
		glm::vec3 mMax{ 0.f, 0.f, 0.f };

		glm::vec3 mBasePosition{ 0.f, 0.f, 0.f };
		glm::vec3 mBaseScale{ 1.f, 1.f, 1.f };
	};

	struct Asset_DEPRECATED {
		MeshData meshData;
		MaterialComponent_DEPRECATED material;
	};

	class Loader {
		friend Engine;

		public:

			Loader() = default;
			~Loader() = default;
			Loader(const Loader&) = delete;
			Loader& operator=(const Loader&) = delete;

			static void init(const std::string &resDir, const std::string &shaderDir);

			static const char* loadFileString(const std::string&);

			static GLTFImporter::Scene loadGltfScene(const std::string& fileName);

			/* Load Mesh pointer from an .obj file */
			static MeshData loadMesh_DEPRECATED(const std::string &, bool = false);
			static std::vector<Asset_DEPRECATED> loadMultiAsset_DEPRECATED(const std::string &);

			/* Retrieve Texture pointer from an image file */
			static Texture* loadTexture(const std::string &, TextureFormat);
			static Texture* loadTexture(const std::string &, const std::vector<std::string> &);

		private:
			/* Optionally resize mesh vertex buffers so all the vertices are [-1, 1] */
			static void _findMetaData_DEPRECATED(MeshData& meshData, std::vector<float>& verts, bool doResize);

			/* Load a single texture file */
			static uint8_t* _loadTextureData(int&, int&, int&, const std::string&, TextureFormat, bool = true);
			static void _cleanTextureData(uint8_t*);

			/* Private members */
			static std::string APP_RES_DIR;
			static std::string APP_SHADER_DIR;
			static std::string ENGINE_RES_DIR;
			static std::string ENGINE_SHADER_DIR;
	};
}