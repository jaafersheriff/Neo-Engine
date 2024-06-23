#pragma once

#include "ECS/Component/Component.hpp"

namespace neo {

	START_COMPONENT(MouseRayComponent);
		glm::vec3 mPosition;
		glm::vec3 mDirection;
	END_COMPONENT();
}
