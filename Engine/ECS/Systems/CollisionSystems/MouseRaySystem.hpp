#pragma once

#include "ECS/Systems/System.hpp"

namespace neo {

	class MouseRaySystem : public System {

	public:
		MouseRaySystem() 
			: System("MouseRay System")
		{}
		virtual void update(ECS&) override;
	};
}
