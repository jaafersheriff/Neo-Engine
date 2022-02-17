#pragma once

#include "DemoInfra/IDemo.hpp"

using namespace neo;

class VFC : public IDemo {
public:
	virtual IDemo::Config getConfig() const override;
	virtual void init(ECS& ecs) override;
};
