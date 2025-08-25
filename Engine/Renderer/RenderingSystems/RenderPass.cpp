#include "Renderer/pch.hpp"

#include "RenderPass.hpp"

namespace neo {
	void RenderPasses::clear(FramebufferHandle target, types::framebuffer::AttachmentBits clearFlags, glm::vec4 clearColor, std::optional<std::string> debugName) {
		mPasses.emplace_back(ClearPass{
			target,
			clearFlags,
			clearColor,
			debugName
		});
	}

	void RenderPasses::declarePass(FramebufferHandle target, glm::uvec2 viewport, DrawFunction draw, std::optional<std::string> debugName) {
		mPasses.emplace_back(RenderPass{
			target,
			viewport,
			draw,
			debugName
		});
	}

	void RenderPasses::_execute(const ResourceManagers& resourceManagers, const ECS& ecs) {
		for (const auto& pass : mPasses) {
			util::visit(pass,
				[&](const RenderPass& renderPass) {
					// TODO: Debug string?
					if (!resourceManagers.mFramebufferManager.isValid(renderPass.mTarget)) {
						NEO_LOG_W("Unable to resolve target, skipping pass %s", renderPass.mDebugName.value_or("").c_str());
						return;
					}
					resourceManagers.mFramebufferManager.resolve(renderPass.mTarget).bind();
					glViewport(0, 0, renderPass.mViewport.x, renderPass.mViewport.y);
					renderPass.mDrawFunction(resourceManagers, ecs);
					glFlush();
				},
				[&](const ClearPass& clearPass) {
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