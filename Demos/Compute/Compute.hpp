#pragma once

#include "DemoInfra/IDemo.hpp"

using namespace neo;

namespace Compute {
	class Demo : public IDemo {
	public:
		virtual IDemo::Config getConfig() const override;
		virtual void init(ECS& ecs, ResourceManagers& resourceManagers) override;
		virtual void update(ECS& ecs, ResourceManagers& resourceManagers) override;
		virtual void render(RenderPasses& renderPasses, const ResourceManagers& resourceManagers, const ECS& ecs, const TextureHandle& outputColor, const TextureHandle& outputDepth) override;
		virtual void imGuiEditor(ECS& ecs, ResourceManagers& resourceManagers) override;
	private:
		float mSpriteSize = 0.2f;
		glm::vec3 mSpriteColor = glm::vec3(0.67f, 1.f, 0.55f);
	};
}
