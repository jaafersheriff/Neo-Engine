#pragma once

#include "DemoInfra/IDemo.hpp"

using namespace neo;

namespace Metaballs {
	class Demo : public IDemo {
	public:
		virtual IDemo::Config getConfig() const override;
		virtual void init(ECS& ecs) override;
		virtual void imGuiEditor(ECS& ecs) override;
	};
}
