#pragma once

#include "ECS/Systems/System.hpp"

namespace neo {

	class RotationSystem : public System {

	public:
		RotationSystem() :
			System("Rotation System")
		{}

		virtual void update(ECS& ecs) override;

	};
}