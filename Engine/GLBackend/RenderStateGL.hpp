#pragma once

#include "HAL/RenderState.hpp"

namespace neo {
	void applyRenderState(const RenderState& renderState, const glm::uvec2& viewport);
}