# pragma once

#include "ResourceManager/ResourceManagers.hpp"
#include <glm/glm.hpp>

namespace neo {
	class ECS;

	class RenderPasses {
		friend class Renderer;
	public:
		void clear(FramebufferHandle target, types::framebuffer::AttachmentBits clearFlags, glm::vec4 clearColor = glm::vec4(0.f, 0.f, 0.f, 1.f));

		using DrawFunction = std::function<void(const ResourceManagers& resourceManagers, const ECS& ecs)>;
		void declarePass(FramebufferHandle target, glm::uvec2 viewport, DrawFunction draw);

	private:
		void _execute(const ResourceManagers& resourceManagers, const ECS& ecs);

		struct RenderPass {
			FramebufferHandle mTarget; 
			glm::uvec2 mViewport; 
			DrawFunction mDrawFunction;
		};
		struct ClearPass {
			FramebufferHandle mTarget;
			types::framebuffer::AttachmentBits mClearFlags; 
			glm::vec4 mClearColor;
		};
		std::vector<std::variant<RenderPass, ClearPass>> mPasses;
	};
}