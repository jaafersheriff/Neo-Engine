#pragma once

#include "ECS/Component/Component.hpp"

namespace neo {
#define CSM_CAMERA_COUNT 3

	START_COMPONENT(CSMCameraComponent);
	CSMCameraComponent() {}

	float mSliceDepth = 0.f; // Normalized (linear) distance to camera
	
	virtual int getLod() const = 0;

	virtual void imGuiEditor() override {
		ImGui::Text("Slice Depth: %0.2f", mSliceDepth);
	};
	END_COMPONENT();

	// Really shouldn't be using inheritance like this
	struct CSMCamera0Component : public CSMCameraComponent {
		CSMCamera0Component()
			: CSMCameraComponent()
		{}
		const char* mName = "CSMCamera0Component";

		virtual int getLod() const override { return 0; }
	};
	struct CSMCamera1Component : public CSMCameraComponent {
		CSMCamera1Component()
			: CSMCameraComponent()
		{}
		const char* mName = "CSMCamera1Component";

		virtual int getLod() const override { return 1; }
	};
	struct CSMCamera2Component : public CSMCameraComponent {
		CSMCamera2Component()
			: CSMCameraComponent()
		{}
		const char* mName = "CSMCamera2Component";

		virtual int getLod() const override { return 2; }
	};
}