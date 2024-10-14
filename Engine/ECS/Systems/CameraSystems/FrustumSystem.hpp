#pragma once

#include "ECS/Systems/System.hpp"

namespace neo {

	class FrustumSystem : public System {

	public:
		FrustumSystem() :
			System("Frustum System")
		{}

		virtual void update(ECS& ecs, const ResourceManagers& resourceManagers) override;
	};
}
