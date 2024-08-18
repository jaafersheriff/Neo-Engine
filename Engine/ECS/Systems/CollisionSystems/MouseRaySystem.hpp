#pragma once

#include "ECS/Systems/System.hpp"

namespace neo {

	class MouseRaySystem : public System {

	public:
		MouseRaySystem() 
			: System("MouseRay System")
		{}
		virtual void update(ECS&) override;
	private:
		// This is owned by the Engine...
		// It shouldn't be stateful, and tbh should be moved somewhere more private
	};
}
