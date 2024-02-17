//
//	Many useful helper functions for GLSL shaders - gleaned from various sources including orange book
//	Created by zwood on 2/21/10.
//	Modified by sueda 10/15/15.
//

#pragma once

#include <string>

namespace neo {

	enum class ShaderStage {
		VERTEX,
		FRAGMENT,
		GEOMETRY,
		TESSELLATION_CONTROL,
		TESSELLATION_EVAL,
		COMPUTE
	};

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
		int32_t getGLShaderStage(ShaderStage type);
		void checkFrameBuffer();
		void printProgramInfoLog(uint32_t program);
		void printShaderInfoLog(uint32_t shader);
	}

#ifdef DEBUG_MODE
#define CHECK_GL_FRAMEBUFFER() do {GLHelper::checkFrameBuffer(); } while(0)
#else
#define CHECK_GL_FRAMEBUFFER()
#endif
}
