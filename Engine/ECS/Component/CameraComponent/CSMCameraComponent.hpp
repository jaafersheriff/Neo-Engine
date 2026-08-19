#pragma once

#include "ECS/Component/Component.hpp"

namespace neo {
#define CSM_CAMERA_COUNT 3

	START_COMPONENT(CSMCameraComponent);
	CSMCameraComponent() {}
	
	virtual int getLod() const = 0;
	void imGuiEditor() {};

	END_COMPONENT();

	// Really shouldn't be using inheritance like this
	struct CSMCamera0Component : public CSMCameraComponent {
		CSMCamera0Component()
			: CSMCameraComponent()
		{}
		static constexpr const char* kName = "CSMCamera0Component";

		virtual int getLod() const override { return 0; }
	};
	struct CSMCamera1Component : public CSMCameraComponent {
		CSMCamera1Component()
			: CSMCameraComponent()
		{}
		static constexpr const char* kName = "CSMCamera1Component";

		virtual int getLod() const override { return 1; }
	};
	struct CSMCamera2Component : public CSMCameraComponent {
		CSMCamera2Component()
			: CSMCameraComponent()
		{}
		static constexpr const char* kName = "CSMCamera2Component";

		virtual int getLod() const override { return 2; }
	};
}