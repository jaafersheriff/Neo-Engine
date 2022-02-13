#pragma once

#include "DemoInfra/IDemo.hpp"

using namespace neo;

class BaseDemo : public IDemo {
public:
	virtual IDemo::Config getConfig() const override;
	virtual void init(ECS& ecs) override;
	virtual void update(ECS& ecs) override;
	virtual void destroy() override;

};
