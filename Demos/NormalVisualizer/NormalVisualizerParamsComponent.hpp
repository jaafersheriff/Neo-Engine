#pragma once

#include "ECS/Component/Component.hpp"


namespace NormalVisualizer {

	// What this demo's ImGui editor writes and its render() reads. A component so the frame
	// clone carries it to the render thread - the editor touches the live ECS, render only ever
	// sees the copy.
	START_COMPONENT(NormalVisualizerParamsComponent);
		float mMagnitude = 0.08f;
	END_COMPONENT();
}
