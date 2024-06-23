#include "STBIImageData.hpp"

#pragma warning(push)
#include <stb_image.h>
#pragma warning(pop)

namespace neo {
	STBImageData::STBImageData(const char* _filePath, types::texture::BaseFormats baseFormat, types::ByteFormats byteFormat, bool flip) {
		mFilePath = _filePath;
		stbi_set_flip_vertically_on_load(flip);
		int _components;
		if (byteFormat == types::ByteFormats::UnsignedByte) {
			mData = stbi_load(mFilePath.c_str(), &mWidth, &mHeight, &_components, baseFormat == types::texture::BaseFormats::RGBA ? STBI_rgb_alpha : STBI_rgb);
		}
		else if (byteFormat == types::ByteFormats::UnsignedShort) {
			mData = reinterpret_cast<uint8_t*>(stbi_load_16(mFilePath.c_str(), &mWidth, &mHeight, &_components, baseFormat == types::texture::BaseFormats::RGBA ? STBI_rgb_alpha : STBI_rgb));
		}
		else if (byteFormat == types::ByteFormats::Float) {
			mData = reinterpret_cast<uint8_t*>(stbi_loadf(mFilePath.c_str(), &mWidth, &mHeight, &_components, baseFormat == types::texture::BaseFormats::RGBA ? STBI_rgb_alpha : STBI_rgb));
		}
		else {
			NEO_FAIL("Invalid byte format for stbi");
		}
	}
	STBImageData::~STBImageData() {
		stbi_image_free(mData);
	}
}
