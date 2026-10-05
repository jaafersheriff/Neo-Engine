#pragma once

#include <glm/glm.hpp>

#include <optional>

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
	struct BlendFuncAlpha {
		BlendFuncSrc mBlendSrc = BlendFuncSrc::One;
		BlendFuncDst mBlendDst = BlendFuncDst::OneMinusSrcAlpha;
	};
	struct BlendState {
		BlendEquation mBlendEquation  = BlendEquation::Add;
		BlendFuncSrc mBlendSrc = BlendFuncSrc::Alpha;
		BlendFuncDst mBlendDst = BlendFuncDst::OneMinusSrcAlpha;
		glm::vec4 mBlendColor = glm::vec4(0.f);
		// Unset means alpha blends with the color factors
		std::optional<BlendFuncAlpha> mBlendAlpha = std::nullopt;
	};


	enum class PolygonMode {
		Fill,
		Line,
		Point
	};

	struct RenderState {
		// Default render state
		std::optional<DepthState> mDepthState = DepthState{
			DepthFunc::Less,
			true
		};
		std::optional<CullFace> mCullFace = CullFace::Back;
		std::optional<BlendState> mBlendState = std::nullopt;
		PolygonMode mPolygonMode = PolygonMode::Fill;
		bool mWireframeable = true;
		// x, y, w, h from the bottom left
		std::optional<glm::ivec4> mScissor = std::nullopt;
	};

	constexpr static RenderState sDisableDepthState = RenderState {
		std::nullopt,
		CullFace::Back,
		std::nullopt,
		PolygonMode::Fill,
		true
	};

	constexpr static RenderState sBlitRenderState = RenderState {
		std::nullopt,
		CullFace::Back,
		std::nullopt,
		PolygonMode::Fill,
		false
	};



}