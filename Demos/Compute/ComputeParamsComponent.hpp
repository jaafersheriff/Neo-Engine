#pragma once

#include "ECS/Component/Component.hpp"

#include <glm/glm.hpp>

namespace Compute {

	START_COMPONENT(ComputeParamsComponent);
		float mSpriteSize = 0.2f;
		glm::vec3 mSpriteColor = glm::vec3(0.67f, 1.f, 0.55f);
	END_COMPONENT();
}
