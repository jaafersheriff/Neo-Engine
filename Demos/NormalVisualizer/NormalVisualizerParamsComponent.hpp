#pragma once

#include "ECS/Component/Component.hpp"


namespace NormalVisualizer {

	START_COMPONENT(NormalVisualizerParamsComponent);
		float mMagnitude = 0.08f;
	END_COMPONENT();
}
