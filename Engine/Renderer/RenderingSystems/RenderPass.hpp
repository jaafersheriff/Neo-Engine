# pragma once

#include "ResourceManager/ResourceManagers.hpp"
#include <glm/glm.hpp>

namespace neo {
	class ECS;

	class RenderPasses {
		friend class Renderer;
	public:
		void clear(FramebufferHandle target, types::framebuffer::AttachmentBits clearFlags, glm::vec4 clearColor = glm::vec4(0.f, 0.f, 0.f, 1.f), std::optional<std::string> debugName = std::nullopt);

		using DrawFunction = std::function<void(const ResourceManagers& resourceManagers, const ECS& ecs)>;
		void renderPass(FramebufferHandle target, glm::uvec2 viewport, DrawFunction draw, std::optional<std::string> debugName = std::nullopt);

		void computePass(DrawFunction draw, std::optional<std::string> debugName = std::nullopt);

	private:
		void _execute(const ResourceManagers& resourceManagers, const ECS& ecs);

		struct ComputePass {
			DrawFunction mDrawFunction;
			std::optional<std::string> mDebugName;
		};
		struct RenderPass {
			FramebufferHandle mTarget; 
			glm::uvec2 mViewport; 
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