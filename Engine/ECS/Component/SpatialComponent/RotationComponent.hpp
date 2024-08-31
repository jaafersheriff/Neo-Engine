#pragma once

#include "ECS/Component/Component.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"

#include <ext/imgui_incl.hpp>

namespace neo {

	START_COMPONENT(RotationComponent);
		RotationComponent(glm::vec3 s)
			: mSpeed(s)
		{}
		glm::vec3 mSpeed;

		virtual void imGuiEditor() override {
			ImGui::SliderFloat3("Speed", &mSpeed[0], -5.f, 5.f);
		}

	END_COMPONENT();
}
