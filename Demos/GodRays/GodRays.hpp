#pragma once

#include "DemoInfra/IDemo.hpp"

using namespace neo;

namespace GodRays {
	class Demo : public IDemo {
	public:
		virtual IDemo::Config getConfig() const override;
		virtual void init(ECS& ecs, Renderer& renderer) override;
		virtual void imGuiEditor(ECS& ecs) override;
	};
}
