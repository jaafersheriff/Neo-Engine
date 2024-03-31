#pragma once

#include "DemoInfra/IDemo.hpp"

using namespace neo;

namespace neo {
	class Texture;
}

namespace Sponza {
	class Demo : public IDemo {
	public:
		virtual IDemo::Config getConfig() const override;
		virtual void init(ECS& ecs, ResourceManagers& resourceManagers) override;
		virtual void render(const ResourceManagers& resourceManagers, const ECS& ecs, Framebuffer& backbuffer) override;
		virtual void imGuiEditor(ECS& ecs) override;

	private:
		void _forwardShading(const ResourceManagers& resourceManagers, const ECS& ecs, FramebufferHandle sceneTarget, FramebufferHandle shadowMap, glm::uvec2 viewport);
		void _deferredShading(const ResourceManagers& resourceManagers, const ECS& ecs, FramebufferHandle sceneTarget, glm::uvec2 targetSize, FramebufferHandle shadowMap);
		bool mDrawShadows = true;

		bool mDeferredShading = false;
		int mPointLightCount = 20;
		float mLightDebugRadius = 0.f;
		bool mDrawAO = false;
		float mAORadius = 0.4f;
		float mAOBias = 0.7f;
	};
}
