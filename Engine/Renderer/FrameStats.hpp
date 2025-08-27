
#pragma once

namespace neo {
	// This should be moved to its own file/service locator..?
	struct FrameStats {
		uint32_t mNumDraws = 0;
		uint32_t mNumPrimitives = 0;
		uint32_t mNumUniforms = 0;
		uint32_t mNumSamplers = 0;
		float mGPUTime = 0.f;
		std::vector<std::string> mRenderPasses;
	};
}
