#pragma once

#include "Renderer/Types.hpp"
#include <string>

namespace neo {
	struct STBImageData {
		STBImageData(const char* _filePath, types::texture::BaseFormats baseFormat, types::ByteFormats byteFormat, bool flip);
		~STBImageData();

		operator bool() const {
			return mData != nullptr && mWidth > 0 && mHeight > 0;
		}

		std::string mFilePath;
		uint8_t* mData = nullptr;
		int mWidth = 0;
		int mHeight = 0;
	};
}