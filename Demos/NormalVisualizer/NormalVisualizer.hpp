#pragma once

#include "DemoInfra/IDemo.hpp"

using namespace neo;

namespace NormalVisualizer {
	class Demo : public IDemo {
	public:
		virtual IDemo::Config getConfig() const override;
		virtual void init(ECS& ecs, Renderer& renderer) override;
	};
}
