#pragma once

#include "Renderer/RenderingSystems/RenderState.hpp"

namespace neo {
	void applyRenderState(const RenderState& renderState, const glm::uvec2& viewport, bool wireframeOverride = false);
}