#pragma once

#include "ECS/Component/Component.hpp"

#include <imgui.h>

namespace neo {
	START_COMPONENT(WireframeRenderComponent);
		glm::vec3 mColor = glm::vec3(1.f);

		WireframeRenderComponent(glm::vec3 color = glm::vec3(1.f)) 
			: mColor(color)
		{}

		virtual void imGuiEditor() override {
			ImGui::ColorEdit3("Color", &mColor[0]);
		}
	END_COMPONENT();
}