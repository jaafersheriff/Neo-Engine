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
		
		/*uint32_t getGLByteFormat(types::ByteFormats format);
		void checkFrameBuffer();
		void printProgramInfoLog(uint32_t program);
		void printShaderInfoLog(uint32_t shader);*/
	}

}
