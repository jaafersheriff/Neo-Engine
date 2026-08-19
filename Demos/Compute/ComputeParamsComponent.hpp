#pragma once

#include "ECS/Component/Component.hpp"

#include <glm/glm.hpp>

namespace Compute {

	// What this demo's ImGui editor writes and its render() reads. A component so the frame
	// clone carries it to the render thread - the editor touches the live ECS, render only ever
	// sees the copy.
	START_COMPONENT(ComputeParamsComponent);
		float mSpriteSize = 0.2f;
		glm::vec3 mSpriteColor = glm::vec3(0.67f, 1.f, 0.55f);
	END_COMPONENT();
}
