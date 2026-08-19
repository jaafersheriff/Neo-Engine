#pragma once

#include "ECS/Component/Component.hpp"


namespace CSM {

	// What this demo's ImGui editor writes and its render() reads. A component so the frame
	// clone carries it to the render thread - the editor touches the live ECS, render only ever
	// sees the copy.
	START_COMPONENT(CSMParamsComponent);
		bool mDebugView = true;
		bool mDrawCascadeLines = true;
		bool mDrawCascadeSpheres = false;
	END_COMPONENT();
}
