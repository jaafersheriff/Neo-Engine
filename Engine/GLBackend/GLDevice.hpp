#pragma once

#include "HAL/FrameStats.hpp"
#include "HAL/RenderDetails.hpp"

namespace neo {

	// Everything here must be called on the thread that owns the GL context
	namespace GLDevice {

		void init(RendererDetails& details);

		void applyDefaultState();
		void collectGpuProfile();

		// The details populated during init
		const RendererDetails& details();
		// Runtime counters
		FrameStats& stats();

	}

}
