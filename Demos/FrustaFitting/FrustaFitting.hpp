#pragma once

#include "DemoInfra/IDemo.hpp"

using namespace neo;

class FrustaFitting : public IDemo {
public:
	virtual IDemo::Config getConfig() const override;
	virtual void init(ECS& ecs) override;
};
