#include "STBIImageData.hpp"

#pragma warning(push)
#include <stb_image.h>
#pragma warning(pop)

namespace neo {
	STBImageData::STBImageData(const char* _filePath, types::texture::BaseFormats baseFormat, bool flip) {
		mFilePath = _filePath;
		stbi_set_flip_vertically_on_load(flip);
		int _components;
		mData = stbi_load(mFilePath.c_str(), &mWidth, &mHeight, &_components, baseFormat == types::texture::BaseFormats::RGBA ? STBI_rgb_alpha : STBI_rgb);

	}
	STBImageData::~STBImageData() {
		stbi_image_free(mData);
	}
}
