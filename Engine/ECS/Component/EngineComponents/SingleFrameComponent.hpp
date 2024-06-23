#pragma once

#include "ECS/Component/Component.hpp"

namespace neo {

	// Deletes the entity on end frame
	START_COMPONENT(SingleFrameComponent);
	END_COMPONENT();
}