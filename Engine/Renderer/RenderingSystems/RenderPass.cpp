#include "Renderer/pch.hpp"

#include "Renderer/FrameStats.hpp"
#include "RenderPass.hpp"

namespace neo {
	namespace {
		void _applyState(const RenderState& renderState) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, 0);
			glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
			glBindVertexArray(0);
			glUseProgram(0);

			NEO_UNUSED(renderState);
			NEO_FAIL("Apply the state");
		}
	}
	void RenderPasses::clear(FramebufferHandle target, types::framebuffer::AttachmentBits clearFlags, glm::vec4 clearColor, std::optional<std::string> debugName) {
		mPasses.emplace_back(ClearPass{
			target,
			clearFlags,
			clearColor,
			debugName
		});
	}

	void RenderPasses::renderPass(FramebufferHandle target, glm::uvec2 viewport, RenderState renderState, DrawFunction draw, std::optional<std::string> debugName) {
		renderState.mWireframe = mWireframeOverride;
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

	void RenderPasses::_execute(FrameStats& renderStats, const ResourceManagers& resourceManagers, const ECS& ecs) {
		renderStats.mRenderPasses.clear();
		TRACY_GPU();
		for (const auto& pass : mPasses) {
			util::visit(pass,
				[&](const ComputePass& computePass) {
					renderStats.mRenderPasses.emplace_back(computePass.mDebugName.value_or("Nameless compute pass"));

					computePass.mDrawFunction(resourceManagers, ecs);
				},
				[&](const RenderPass& renderPass) {
					renderStats.mRenderPasses.emplace_back(renderPass.mDebugName.value_or("Nameless compute pass"));

					if (!resourceManagers.mFramebufferManager.isValid(renderPass.mTarget)) {
						NEO_LOG_W("Unable to resolve target, skipping pass %s", renderPass.mDebugName.value_or("").c_str());
						return;
					}
					resourceManagers.mFramebufferManager.resolve(renderPass.mTarget).bind();
					glViewport(0, 0, renderPass.mViewport.x, renderPass.mViewport.y);

					_applyState(renderPass.mRenderState);
					renderPass.mDrawFunction(resourceManagers, ecs);
				},
				[&](const ClearPass& clearPass) {
					renderStats.mRenderPasses.emplace_back(clearPass.mDebugName.value_or("Nameless render pass"));

					TRACY_GPUN("Clear");
					if (!resourceManagers.mFramebufferManager.isValid(clearPass.mTarget)) {
						NEO_LOG_W("Unable to resolve target, skipping clear %s", clearPass.mDebugName.value_or("").c_str());
						return;
					}
					resourceManagers.mFramebufferManager.resolve(clearPass.mTarget).bind();
					resourceManagers.mFramebufferManager.resolve(clearPass.mTarget).clear(clearPass.mClearColor, clearPass.mClearFlags);

				},
				[&](auto) { static_assert(always_false_v<T>, "non-exhaustive visitor!"); }
			);
		}
	}
}