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

	time_t Loader::getFileModTime(const std::string& fileName) {
		time_t ret = 0;
		bool success = _operate(fileName, [&ret](const char* fullPath) {
			ret = util::getFileModTime(fullPath);
		});
		if (!success) {
			NEO_LOG_E("Unable to get mod time for %s", fileName.c_str());
		}
		return ret;
	}

	const char* Loader::loadFileString(const std::string& fileName) {
		const char* ret = nullptr;
		bool success = _operate(fileName, [&ret](const char* fullPath) {
			// This does a malloc..
			ret = util::textFileRead(fullPath);
		});

		if (!success) {
			NEO_LOG_E("Unable to find string file %s", fileName.c_str());
		}
		return ret;
	}

	GLTFImporter::Scene Loader::loadGltfScene(ResourceManagers& resourceManagers, const std::string& fileName, glm::mat4 baseTransform) {
		GLTFImporter::Scene ret;
		bool success = _operate(fileName, [&](const char* fullPath) {
			ret = GLTFImporter::loadScene(fullPath, baseTransform, resourceManagers);
		});
		if (!success) {
			NEO_LOG_E("Unable to find GLTF scene %s", fileName.c_str());
		}
		return ret;
	}

	bool Loader::_operate(const std::string& fileName, std::function<void(const char*)> callback) {
		char fullPath[512];
		auto searchDir = [&fullPath, fileName, callback](const std::string& dir) {
			sprintf(fullPath, "%s%s\0", dir.c_str(), fileName.c_str());
			if (util::fileExists(fullPath)) {
				callback(fullPath);
				return true;
			}
			return false;
		};
		return searchDir(APP_RES_DIR) || searchDir(APP_SHADER_DIR) || searchDir(ENGINE_RES_DIR) || searchDir(ENGINE_SHADER_DIR);
	}
}
