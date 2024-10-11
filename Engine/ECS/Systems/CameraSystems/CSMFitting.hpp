#pragma once

#include "ECS/Systems/System.hpp"

namespace neo {

	class CSMFitting : public neo::System {

	public:

		CSMFitting() :
			neo::System("CSM Fitting System") {
		}

		virtual void update(neo::ECS& ecs) override;
	};
}
