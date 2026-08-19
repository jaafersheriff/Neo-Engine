#pragma once

#include "ECS/Component/Component.hpp"

namespace neo {

	START_COMPONENT(RendererParamsComponent);
		bool mShowBoundingBoxes = false;
		bool mWireframe = false;
	END_COMPONENT();
}
