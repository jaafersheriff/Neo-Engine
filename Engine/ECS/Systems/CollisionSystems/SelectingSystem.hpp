#pragma once

#include "ECS/Systems/System.hpp"

namespace neo {

	class SelectingSystem : public System {

	public:
		SelectingSystem() 
			: System("Selecting System")
		{}


		virtual void update(ECS&) override;
	};
}