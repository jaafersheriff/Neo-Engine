#include "Renderer/pch.hpp"

#include "RenderPass.hpp"

namespace neo {
	void RenderPasses::clear(FramebufferHandle target, types::framebuffer::AttachmentBits clearFlags, glm::vec4 clearColor) {
		mPasses.emplace_back(ClearPass{
			target,
			clearFlags,
			clearColor
		});
	}

	void RenderPasses::declarePass(FramebufferHandle target, glm::uvec2 viewport, DrawFunction draw) {
		mPasses.emplace_back(RenderPass{
			target,
			viewport,
			draw
		});
	}

	void RenderPasses::_execute(const ResourceManagers& resourceManagers, const ECS& ecs) {
		for (const auto& pass : mPasses) {
			util::visit(pass,
				[&](const RenderPass& renderPass) {
					// TODO: Debug string?
					if (!resourceManagers.mFramebufferManager.isValid(renderPass.mTarget)) {
						NEO_LOG_E("Unable to resolve target, skipping pass");
						return;
					}
					resourceManagers.mFramebufferManager.resolve(renderPass.mTarget).bind();
					glViewport(0, 0, renderPass.mViewport.x, renderPass.mViewport.y);
					renderPass.mDrawFunction(resourceManagers, ecs);
				},
				[&](const ClearPass& clearPass) {
					if (!resourceManagers.mFramebufferManager.isValid(clearPass.mTarget)) {
						NEO_LOG_E("Unable to resolve target, skipping clear");
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