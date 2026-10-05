#pragma once

#include "HAL/RenderDetails.hpp"

namespace neo {

	// Everything here must be called on the thread that owns the GL context
	namespace GLDevice {

		void init(RendererDetails& details);
		void applyDefaultState();
	}

}
