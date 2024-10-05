#pragma once

#include "ECS/Systems/System.hpp"

namespace CSM {

	class CSMFitting : public neo::System {

	public:

		CSMFitting() :
			neo::System("FrustaFitting System") {
		}

		virtual void update(neo::ECS& ecs) override;
	};
}
