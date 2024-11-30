#pragma once

#include "ECS/Systems/System.hpp"

namespace neo {

	class CSMFittingSystem : public neo::System {

	public:

		CSMFittingSystem() :
			neo::System("CSM Fitting System") {
		}

		virtual void update(neo::ECS& ecs, const ResourceManagers& resourceManagers) override;
	};
}
