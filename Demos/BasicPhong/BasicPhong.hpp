#pragma once

#include "DemoInfra/IDemo.hpp"

using namespace neo;

class BasicPhong : public IDemo {
public:
	BasicPhong();
	~BasicPhong();

	virtual void init(ECS& ecs) override;
	virtual void update(ECS& ecs) override;
	virtual void destroy() override;

};
