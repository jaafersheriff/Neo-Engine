#pragma once

#include "DemoInfra/IDemo.hpp"

using namespace neo;

namespace Base {

	class Demo : public IDemo {
	public:
		virtual IDemo::Config getConfig() const override;
		virtual void init(ECS& ecs, ResourceManagers& resourceManagers) override;
		virtual void update(ECS& ecs, ResourceManagers& resourceManagers) override;
		virtual void render(FrameGraph& fg, const ResourceManagers& resourceManagers, const ECS& ecs, FramebufferHandle backbufferHandle) override;
		virtual void destroy() override;
		virtual void imGuiEditor(ECS& ecs, ResourceManagers& resourceManagers) override;

	};
}
