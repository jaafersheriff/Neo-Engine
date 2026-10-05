#include "Renderer/pch.hpp"

#include "Renderer/FrameStats.hpp"
#include "GLBackend/RenderStateGL.hpp"

#include "RenderPass.hpp"
#include "Util/Log/Log.hpp"
#include "Util/Visit.hpp"

namespace neo {

	void RenderPasses::clear(FramebufferHandle target, types::framebuffer::AttachmentBits clearFlags, glm::vec4 clearColor, std::optional<std::string> debugName) {
		mPasses.emplace_back(ClearPass{
			target,
			clearFlags,
			clearColor,
			debugName
		});
	}

	void RenderPasses::renderPass(FramebufferHandle target, const glm::uvec2& viewport, const RenderState& renderState, DrawFunction draw, std::optional<std::string> debugName) {
		mPasses.emplace_back(RenderPass{
			target,
			viewport,
			renderState,
			draw,
			debugName
		});
	}

	void RenderPasses::computePass(DrawFunction draw, std::optional<std::string> debugName) {
		mPasses.emplace_back(ComputePass{
			draw,
			debugName
		});
	}

	void RenderPasses::blit(FramebufferHandle src, FramebufferHandle dst, const glm::uvec2& dimension, types::framebuffer::AttachmentBits attachments, std::optional<std::string> debugName) {
		mPasses.emplace_back(BlitPass{
			src,
			dst,
			dimension,
			attachments,
			debugName
		});
	}

	void RenderPasses::_execute(const ResourceManagers& resourceManagers, const ECS& ecs, bool wireframe) {

		TRACY_GPU();
		for (const auto& pass : mPasses) {
			util::visit(pass,
				[&](const ComputePass& computePass) {

					computePass.mDrawFunction(resourceManagers, ecs);
				},
				[&](const RenderPass& renderPass) {

					if (!resourceManagers.mFramebufferManager.isValid(renderPass.mTarget)) {
						NEO_LOG_W("Unable to resolve target, skipping pass %s", renderPass.mDebugName.value_or("").c_str());
						return;
					}
					resourceManagers.mFramebufferManager.resolve(renderPass.mTarget).bind();

					RenderState state = renderPass.mRenderState;
					if (wireframe && renderPass.mRenderState.mWireframeable) {
						state.mPolygonMode = PolygonMode::Line;
					}
					applyRenderState(state, renderPass.mViewport);

					renderPass.mDrawFunction(resourceManagers, ecs);
				},
				[&](const ClearPass& clearPass) {

					TRACY_GPUN("Clear");
					if (!resourceManagers.mFramebufferManager.isValid(clearPass.mTarget)) {
						NEO_LOG_W("Unable to resolve target, skipping clear %s", clearPass.mDebugName.value_or("").c_str());
						return;
					}
					resourceManagers.mFramebufferManager.resolve(clearPass.mTarget).bind();
					resourceManagers.mFramebufferManager.resolve(clearPass.mTarget).clear(clearPass.mClearColor, clearPass.mClearFlags);

				},
				[&](const BlitPass& blitPass) {

					TRACY_GPUN("Blit");
					if (!resourceManagers.mFramebufferManager.isValid(blitPass.mSrc) || !resourceManagers.mFramebufferManager.isValid(blitPass.mDst)) {
						NEO_LOG_W("Unable to resolve target, skipping blit %s", blitPass.mDebugName.value_or("").c_str());
						return;
					}
					resourceManagers.mFramebufferManager.resolve(blitPass.mSrc).blit(
						resourceManagers.mFramebufferManager.resolve(blitPass.mDst),
						blitPass.mDimension,
						blitPass.mAttachments
					);

				},
				[&](auto) { static_assert(always_false_v<T>, "non-exhaustive visitor!"); }
			);
		}
	}
}