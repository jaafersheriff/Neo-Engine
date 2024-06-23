
#pragma once

#include "ECS/Component/Component.hpp"
#include "Util/Util.hpp"

namespace neo {

	START_COMPONENT(PointLightComponent);
		glm::vec3 mAttenuation;

		PointLightComponent(const glm::vec3 att = glm::vec3(0.1, 0.001, 0.0001)) 
			: mAttenuation(att) { }

		virtual void imGuiEditor() {
			ImGui::SliderFloat("Attenuation.x", &mAttenuation[0], 0.f, 1.f);
			ImGui::SliderFloat("Attenuation.y", &mAttenuation[1], 0.f, 0.1f);
			ImGui::SliderFloat("Attenuation.z", &mAttenuation[2], 0.f, 0.01f);
		}
	END_COMPONENT();
}
