#pragma once

#include <glm/glm.hpp>

namespace neo {

	using Viewport = glm::uvec4;
	enum class DepthFunc {
		Less
	};
	enum class CullOrder {
		Front,
		Back
	};
	enum class BlendEquation {
		Add
	};
	enum class BlendFactor {
		One,
		Alpha,
		OneMinusAlpha
	};
	struct PassState {
		bool mDepthTest = true;
		DepthFunc mDepthFunc = DepthFunc::Less;
		bool mDepthMask = true;

		bool mCullFace = true;
		CullOrder mCullOrder = CullOrder::Back;

		bool mBlending = false;
		BlendEquation mBlendEquation = BlendEquation::Add;
		BlendFactor mBlendSrcRGB = BlendFactor::Alpha;
		BlendFactor mBlendDstRGB = BlendFactor::OneMinusAlpha;
		BlendFactor mBlendSrcAlpha = BlendFactor::Alpha;
		BlendFactor mBlendDstAlpha = BlendFactor::OneMinusAlpha;

		bool mStencilTest = false;
		bool mScissorTest = false;
	};


}