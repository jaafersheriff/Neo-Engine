#pragma once

#include "ECS/Component/Component.hpp"

namespace neo {
	START_COMPONENT(CSMCameraComponent);
	CSMCameraComponent(float sliceDepth) 
		: mSliceDepth(sliceDepth)
	{}

	float mSliceDepth = 0.f;
	END_COMPONENT();

	// Really shouldn't be using inheritance like this
	struct CSMCamera0Component : public CSMCameraComponent {
		CSMCamera0Component(float sliceDepth)
			: CSMCameraComponent(sliceDepth)
		{}
		const char* mName = "CSMCamera0Component";
	};
	struct CSMCamera1Component : public CSMCameraComponent {
		CSMCamera1Component(float sliceDepth)
			: CSMCameraComponent(sliceDepth)
		{}
		const char* mName = "CSMCamera1Component";
	};
	struct CSMCamera2Component : public CSMCameraComponent {
		CSMCamera2Component(float sliceDepth)
			: CSMCameraComponent(sliceDepth)
		{}
		const char* mName = "CSMCamera2Component";
	};
	struct CSMCamera3Component : public CSMCameraComponent {
		CSMCamera3Component(float sliceDepth)
			: CSMCameraComponent(sliceDepth)
		{}
		const char* mName = "CSMCamera3Component";
	};

}