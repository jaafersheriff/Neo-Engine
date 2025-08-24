# pragma once

#include <glm/glm.hpp>

namespace neo {

	class RenderPasses {
		friend class Renderer;
	public:
		void clear(FramebufferHandle target, types::framebuffer::AttachmentBits clearFlags, glm::vec4 clearColor = glm::vec4(0.f, 0.f, 0.f, 1.f));

		using DrawFunction = std::function<void(const ResourceManagers& resourceManagers, const ECS& ecs)>;
		void declarePass(FramebufferHandle target, glm::uvec2 viewport, DrawFunction draw);
	private:
		void _execute();

		class RenderPass {
		};
		std::vector<RenderPass> mRenderPasses;
	};
}