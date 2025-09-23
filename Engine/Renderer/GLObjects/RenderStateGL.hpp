#pragma once

#include "Renderer/RenderingSystems/RenderState.hpp"

namespace neo {
	void applyRenderState(const RenderState& renderState, bool wireframeOverride = false);
}