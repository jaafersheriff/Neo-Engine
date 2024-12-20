#pragma once

#include <optional>
#include <vector>
#include <functional>

#include <glm/glm.hpp>

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

			static time_t getFileModTime(const std::string& fileName);
			static const char* loadFileString(const std::string&);

			static GLTFImporter::Scene loadGltfScene(ResourceManagers& resourceManagers, const std::string& fileName, glm::mat4 baseTransform = glm::mat4(1.f));

			static std::string APP_RES_DIR;
			static std::string APP_SHADER_DIR;
			static std::string ENGINE_RES_DIR;
			static std::string ENGINE_SHADER_DIR;

		private:
			static bool _operate(const std::string& fileName, std::function<void(const char*)> callback);
	};
}