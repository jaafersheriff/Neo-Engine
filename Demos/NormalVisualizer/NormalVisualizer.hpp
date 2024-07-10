#pragma once

#include "DemoInfra/IDemo.hpp"

using namespace neo;

namespace NormalVisualizer {
	class Demo : public IDemo {
	public:
		virtual IDemo::Config getConfig() const override;
		virtual void init(ECS& ecs, ResourceManagers& resourceManagers) override;
		virtual void render(const ResourceManagers& resourceManagers, const ECS& ecs, Framebuffer& backbuffer) override;
		virtual void imGuiEditor(ECS& ecs, ResourceManagers& resourceManagers) override;

	private:
		float mMagnitude = 0.08f;
	};
}
