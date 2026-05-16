//
//	Many useful helper functions for GLSL shaders - gleaned from various sources including orange book
//	Created by zwood on 2/21/10.
//	Modified by sueda 10/15/15.
//

#pragma once

#include "Renderer/Types.hpp"

#include <string>

namespace neo {

	namespace GLHelper {

		void OpenGLMessageCallback(
			unsigned source,
			unsigned type,
			unsigned id,
			unsigned severity,
			int length,
			const char* message,
			const void* userParam
		);
		uint32_t getGLByteFormat(types::ByteFormats format);
		uint32_t getGLInternalFormat(types::texture::InternalFormats format);
		const char* errorString(unsigned int err);
		void checkError(const char* str);
	}

}
