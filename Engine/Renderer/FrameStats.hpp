
#pragma once

namespace neo {
	// Required to be a POD because thread safety isn't guaranteed (see Renderer::mStats)
	struct FrameStats {
		uint32_t mNumDraws = 0;
		uint32_t mNumPrimitives = 0;
		uint32_t mNumUniforms = 0;
		uint32_t mNumSamplers = 0;
		float mGPUTime = 0.f;
	};
}
