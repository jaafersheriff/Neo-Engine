#pragma once

#include "ECS/Systems/System.hpp"

namespace neo {

	class SelectingSystem : public System {

	public:
		SelectingSystem() 
			: System("Selecting System")
		{}

		virtual void update(ECS&) override;

	private:
		// This is owned by the Engine...
		// It shouldn't be stateful, and tbh should be moved somewhere more private
	};
}