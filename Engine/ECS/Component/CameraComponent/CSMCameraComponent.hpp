#pragma once

#include "ECS/Component/Component.hpp"

namespace neo {
	START_COMPONENT(CSMCameraComponent);
	virtual int getLod() const = 0; // Virtual so you can't use this component directly
	float mSliceDepthEnd = 0.f;
	END_COMPONENT();

	// Really shouldn't be using inheritance like this
	struct CSMCamera0Component : public CSMCameraComponent {
		const char* mName = "CSMCamera0Component";
		virtual int getLod() const override {
			return 0;
		}
	};
	struct CSMCamera1Component : public CSMCameraComponent {
		const char* mName = "CSMCamera1Component";
		virtual int getLod() const override {
			return 1;
		}
	};
	struct CSMCamera2Component : public CSMCameraComponent {
		const char* mName = "CSMCamera2Component";
		virtual int getLod() const override {
			return 2;
		}
	};
	struct CSMCamera3Component : public CSMCameraComponent {
		const char* mName = "CSMCamera3Component";
		virtual int getLod() const override {
			return 3;
		}
	};

}