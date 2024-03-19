#pragma once

#include "ECS/Component/Component.hpp"

#include <imgui.h>

namespace neo {
	struct WireframeShaderComponent : public Component {
		glm::vec3 mColor = glm::vec3(1.f);

		WireframeShaderComponent(glm::vec3 color = glm::vec3(1.f)) 
			: Component()
			, mColor(color)
		{}

		virtual std::string getName() const override {
			return "WireframeShaderComponent";
		}

		virtual void imGuiEditor() override {
			ImGui::ColorEdit3("Color", &mColor[0]);
		}
	};
}