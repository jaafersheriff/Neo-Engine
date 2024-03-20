#pragma once

#include <optional>
#include <vector>

namespace neo {

	class Engine;
	class Mesh;
	class Texture;
	struct TextureFormat;
	class ResourceManagers;
	namespace GLTFImporter {
		struct Scene;
	}

	class Loader {
		friend Engine;

		public:

			Loader() = default;
			~Loader() = default;
			Loader(const Loader&) = delete;
			Loader& operator=(const Loader&) = delete;

			static void init(const std::string &resDir, const std::string &shaderDir);

			static const char* loadFileString(const std::string&);

			static GLTFImporter::Scene loadGltfScene(ResourceManagers& resourceManagers, const std::string& fileName, glm::mat4 baseTransform = glm::mat4(1.f));

			/* Retrieve Texture pointer from an image file */
			static Texture* loadTexture(const std::string &, TextureFormat format);
			static Texture* loadTexture(const std::string &, const std::vector<std::string> &);

			static std::string APP_RES_DIR;
			static std::string APP_SHADER_DIR;
			static std::string ENGINE_RES_DIR;
			static std::string ENGINE_SHADER_DIR;

		private:
			/* Load a single texture file */
			static uint8_t* _loadTextureData(int&, int&, int&, const std::string&, TextureFormat format, bool = true);
			static void _cleanTextureData(uint8_t*);

	};
}