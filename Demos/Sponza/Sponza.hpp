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
		virtual void init(ECS& ecs) override;
		virtual void render(const ECS& ecs, Framebuffer& backbuffer) override;
		virtual void imGuiEditor(ECS& ecs) override;

	private:
		void _forwardShading(const ECS& ecs, Framebuffer& sceneTarget, Texture* shadowMap);
		void _deferredShading(const ECS& ecs, Framebuffer& sceneTarget, glm::uvec2 targetSize, Texture* shadowMap);
		bool mDeferredShading = false;
		int mPointLightCount = 20;
		float mLightDebugRadius = 0.f;
	};
}
