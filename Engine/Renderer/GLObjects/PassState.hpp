#pragma once

#include "Renderer/Types.hpp"

#include <glm/glm.hpp>

namespace neo {

	using Viewport = glm::uvec4;
	struct PassState {

		bool mDepthTest = true;
		types::passState::DepthFunc mDepthFunc = types::passState::DepthFunc::Less;
		bool mDepthMask = true;

		bool mCullFace = true;
		types::passState::CullOrder mCullOrder = types::passState::CullOrder::Back;

		bool mBlending = false;
		types::passState::BlendEquation mBlendEquation = types::passState::BlendEquation::Add;
		types::passState::BlendFactor mBlendSrcRGB = types::passState::BlendFactor::Alpha;
		types::passState::BlendFactor mBlendDstRGB = types::passState::BlendFactor::OneMinusAlpha;
		types::passState::BlendFactor mBlendSrcAlpha = types::passState::BlendFactor::Alpha;
		types::passState::BlendFactor mBlendDstAlpha = types::passState::BlendFactor::OneMinusAlpha;

		bool mStencilTest = false;
		bool mScissorTest = false;
	};


}