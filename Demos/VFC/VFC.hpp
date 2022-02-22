#pragma once

#include "DemoInfra/IDemo.hpp"

using namespace neo;

namespace VFC {
	class Demo : public IDemo {
	public:
		virtual IDemo::Config getConfig() const override;
		virtual void init(ECS& ecs) override;
	};
}
