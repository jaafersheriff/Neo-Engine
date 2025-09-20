# pragma once

#include "ResourceManager/ResourceManagers.hpp"
#include <glm/glm.hpp>

namespace neo {
	class ECS;

	// TODO - these should move to their own file
	enum class DepthFunc {
		Less,
		LessEqual
	};
	struct DepthState {
		DepthFunc mDepthFunc;
		bool mDepthMask;
	};

	// TODO - these should move to their own file
	enum class CullFace {
		Back,
		Front
	};

	// TODO - these should move to their own file
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

	// TODO - these should move to their own file
	struct RenderState {
		// Default render state
		std::optional<DepthState> mDepthState = DepthState{
			DepthFunc::Less,
			true
		};
		std::optional<CullFace> mCullFace = CullFace::Back;
		std::optional<BlendState> mBlendState = std::nullopt;
		bool mWireframe = false;
	};

	class RenderPasses {
		friend class Renderer;
	public:
		void clear(FramebufferHandle target, types::framebuffer::AttachmentBits clearFlags, glm::vec4 clearColor = glm::vec4(0.f, 0.f, 0.f, 1.f), std::optional<std::string> debugName = std::nullopt);

		using DrawFunction = std::function<void(const ResourceManagers& resourceManagers, const ECS& ecs)>;
		void renderPass(FramebufferHandle target, glm::uvec2 viewport, RenderState renderState, DrawFunction draw, std::optional<std::string> debugName = std::nullopt);

		void computePass(DrawFunction draw, std::optional<std::string> debugName = std::nullopt);

	private:
		void _execute(FrameStats& stats, const ResourceManagers& resourceManagers, const ECS& ecs);
		bool mWireframeOverride = false;

		struct ComputePass {
			DrawFunction mDrawFunction;
			std::optional<std::string> mDebugName;
		};
		struct RenderPass {
			FramebufferHandle mTarget; 
			glm::uvec2 mViewport; 
			RenderState mRenderState;
			DrawFunction mDrawFunction;
			std::optional<std::string> mDebugName;
		};
		struct ClearPass {
			FramebufferHandle mTarget;
			types::framebuffer::AttachmentBits mClearFlags; 
			glm::vec4 mClearColor;
			std::optional<std::string> mDebugName;
		};
		std::vector<std::variant<ComputePass, RenderPass, ClearPass>> mPasses;
	};
}