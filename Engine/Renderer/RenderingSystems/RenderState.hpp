#pragma once

#include <glm/glm.hpp>

namespace neo {

	/// Depth
	enum class DepthFunc {
		Less,
		LessEqual
	};
	struct DepthState {
		DepthFunc mDepthFunc;
		bool mDepthMask;
	};

	/// Face culling
	enum class CullFace {
		Back,
		Front
	};


	/// Blending
	enum class BlendEquation {
		Add,
	};
	enum class BlendFuncSrc {
		Alpha,
		One
	};
	enum class BlendFuncDst {
		OneMinusSrcAlpha,
		One
	};
	struct BlendState {
		BlendEquation mBlendEquation  = BlendEquation::Add;
		BlendFuncSrc mBlendSrc = BlendFuncSrc::Alpha;
		BlendFuncDst mBlendDst = BlendFuncDst::OneMinusSrcAlpha;
		glm::vec4 mBlendColor = glm::vec4(0.f);
	};


	struct RenderState {
		// Default render state
		std::optional<DepthState> mDepthState = DepthState{
			DepthFunc::Less,
			true
		};
		std::optional<CullFace> mCullFace = CullFace::Back;
		std::optional<BlendState> mBlendState = std::nullopt;
		bool mWireframeable = true;
	};

	constexpr static RenderState sDisableDepthState = RenderState {
		std::nullopt,
		CullFace::Back,
		std::nullopt,
		true
	};

	constexpr static RenderState sBlitRenderState = RenderState {
		std::nullopt,
		CullFace::Back,
		std::nullopt,
		false
	};



}