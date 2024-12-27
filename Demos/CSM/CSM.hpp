#pragma once

#include "DemoInfra/IDemo.hpp"

using namespace neo;

namespace CSM {
	class Demo : public IDemo {
	public:
		virtual IDemo::Config getConfig() const override;
		virtual void init(ECS& ecs, ResourceManagers& resourceManagers) override;
		virtual void update(ECS& ecs, ResourceManagers& resourceManagers) override;
		virtual void render(const ResourceManagers& resourceManagers, const ECS& ecs, Framebuffer& backbuffer) override;
		virtual void imGuiEditor(ECS& ecs, ResourceManagers& resourceManagers) override;

		bool mDebugView = true;
		bool mDrawCascadeLines = true;
		bool mDrawCascadeSpheres = false;
	};
}
