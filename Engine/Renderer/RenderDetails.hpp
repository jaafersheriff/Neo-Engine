
#pragma once

#include <glm/glm.hpp>
#include <string>

namespace neo {
	struct RendererDetails {
		int mGLMajorVersion = 0;
		int mGLMinorVersion = 0;
		std::string mGLSLVersion = "";
		glm::ivec3 mMaxComputeWorkGroupSize = { 0,0,0 };
		std::string mVendor = "";
		std::string mRenderer = "";
		std::string mShadingLanguage = "";
	};
}
