#pragma once

#include "ECS/Component/Component.hpp"


namespace CSM {

	START_COMPONENT(CSMParamsComponent);
		bool mDebugView = true;
		bool mDrawCascadeLines = true;
		bool mDrawCascadeSpheres = false;
	END_COMPONENT();
}
