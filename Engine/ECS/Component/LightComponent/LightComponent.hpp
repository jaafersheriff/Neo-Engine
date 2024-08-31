#pragma once

#include "ECS/Component/Component.hpp"

#include <ext/imgui_incl.hpp>

namespace neo {

	START_COMPONENT(LightComponent);
		glm::vec3 mColor;
		float mIntensity;

		LightComponent(const glm::vec3& col = glm::vec3(1.f), float intensity = 1.f) 
			: mColor(col)
			, mIntensity(intensity)
		{}

		virtual void imGuiEditor() {
			ImGui::ColorEdit3("Color", &mColor[0]);
			ImGui::SliderFloat("Intensity", &mIntensity, 0.f, 50.f);
		}
	END_COMPONENT();
}
