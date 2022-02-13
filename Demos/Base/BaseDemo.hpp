#pragma once

#include "DemoInfra/IDemo.hpp"

using namespace neo;

class BaseDemo : public IDemo {
public:
	BaseDemo();
	~BaseDemo();

	virtual void init(ECS& ecs) override;
	virtual void update(ECS& ecs) override;
	virtual void destroy() override;

};
