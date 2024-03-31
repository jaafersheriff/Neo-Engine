#include "Loader/pch.hpp"
#include "Loader.hpp"

#include "Renderer/GLObjects/Mesh.hpp"
#include "Renderer/GLObjects/GLHelper.hpp"
#include "Renderer/GLObjects/Framebuffer.hpp"

#include "GLTFImporter.hpp"

#include "Util/Profiler.hpp"

#pragma warning(push)
#include <stb_image.h>
#pragma warning(pop)

namespace neo {

	std::string Loader::APP_RES_DIR = "";
	std::string Loader::APP_SHADER_DIR = "";
	std::string Loader::ENGINE_RES_DIR = "../Engine/res/";
	std::string Loader::ENGINE_SHADER_DIR = "../Engine/shaders/";

	void Loader::init(const std::string &res, const std::string& shaderDir) {
		APP_RES_DIR = res;
		APP_SHADER_DIR = shaderDir;
	}

	const char* Loader::loadFileString(const std::string& fileName) {
		auto load = [](const char* fullPath, const char** ret) {
			if (util::fileExists(fullPath)) {
				// Each of these util::textFileReads does a malloc..
				*ret = util::textFileRead(fullPath);
				return true;
			}
			return false;
		};

		const char* ret;
		// TODO - this should be extended to other load funcs..
		if (load((APP_RES_DIR + fileName).c_str(), &ret)) {
			return ret;
		}
		if (load((APP_SHADER_DIR + fileName).c_str(), &ret)) {
			return ret;
		}
		if (load((ENGINE_RES_DIR + fileName).c_str(), &ret)) {
			return ret;
		}
		if (load((ENGINE_SHADER_DIR + fileName).c_str(), &ret)) {
			return ret;
		}
		NEO_LOG_E("Unable to find string file %s", fileName.c_str());
		return nullptr;
	}

	GLTFImporter::Scene Loader::loadGltfScene(ResourceManagers& resourceManagers, const std::string& fileName, glm::mat4 baseTransform) {
		std::string _fileName = APP_RES_DIR + fileName;
		if (!util::fileExists(_fileName.c_str())) {
			_fileName = ENGINE_RES_DIR + fileName;
		}
		NEO_ASSERT(util::fileExists(_fileName.c_str()), "Unable to find file: %s after checking:\n\t%s\n\t%s\n", fileName.c_str(), APP_RES_DIR.c_str(), ENGINE_RES_DIR.c_str());

		return GLTFImporter::loadScene(_fileName, baseTransform, resourceManagers);
	}
}
