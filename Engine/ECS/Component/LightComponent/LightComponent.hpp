#pragma once

#include "ECS/Component/Component.hpp"

#include <imgui.h>

namespace neo {

	START_COMPONENT(LightComponent);
		glm::vec3 mColor;

		LightComponent(const glm::vec3& col = glm::vec3(1.f)) :
			mColor(col)
		{}

		virtual void imGuiEditor() {
			ImGui::ColorEdit3("Color", &mColor[0]);
		}
	END_COMPONENT();
}
