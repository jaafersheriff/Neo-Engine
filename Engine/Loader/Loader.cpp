#include "Loader/pch.hpp"
#include "Loader.hpp"

#include "Library.hpp"

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

	Texture* Loader::loadTexture(const std::string &fileName, TextureFormat format) {
		TRACY_ZONE();
		/* Create an empty texture if it is not already exist in the library */
		int width, height, components;
		uint8_t* data = _loadTextureData(width, height, components, fileName, format);

		Texture* texture = new Texture(format, glm::uvec2(width, height), data);

		_cleanTextureData(data);

		return texture;

	}

	Texture* Loader::loadTexture(const std::string &name, const std::vector<std::string>& files) {
		TRACY_ZONE();

		NEO_ASSERT(files.size() == 6, "Attempting to create cube map without 6 files");

		std::vector<uint8_t*> data;
		glm::u16vec2 size(UINT16_MAX, UINT16_MAX);
		for (int i = 0; i < 6; i++) {
			int _width, _height, _components;
			data.push_back(_loadTextureData(_width, _height, _components, files[i], {}, false));
			size.x = std::min(size.x, static_cast<uint16_t>(_width));
			size.y = std::min(size.y, static_cast<uint16_t>(_height));
		}

		/* Upload data to GPU and free from CPU */
		TextureFormat format = {
			types::texture::Target::TextureCube,
			types::texture::InternalFormats::RGBA8_UNORM,
		};
		Texture* texture = new Texture(format, size, reinterpret_cast<void**>(data.data()));

		/* Clean */
		for (int i = 0; i < 6; i++) {
			_cleanTextureData(data[i]);
		}

		NEO_LOG_I("Loaded cubemap (%s)", name.c_str());

		return texture;
	}

	uint8_t* Loader::_loadTextureData(int& width, int& height, int& components, const std::string& fileName, TextureFormat format, bool flip) {
		std::string _fileName = APP_RES_DIR + fileName;
		if (!util::fileExists(_fileName.c_str())) {
			_fileName = ENGINE_RES_DIR + fileName;
			NEO_ASSERT(util::fileExists(_fileName.c_str()), "Unable to find file %s", fileName.c_str());
		}

		/* Use stbi if name is an existing file */
		stbi_set_flip_vertically_on_load(flip);
		uint8_t *data = stbi_load(_fileName.c_str(), &width, &height, &components, TextureFormat::deriveBaseFormat(format.mInternalFormat) == types::texture::BaseFormats::RGBA ? STBI_rgb_alpha : STBI_rgb);
		NEO_ASSERT(data, "Error reading texture file");

		NEO_LOG_I("Loaded texture %s [%d, %d]", fileName.c_str(), width, height);

		return data;
	}

	void Loader::_cleanTextureData(uint8_t* data) {
		stbi_image_free(data);
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
