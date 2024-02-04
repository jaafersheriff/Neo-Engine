#pragma once

#include "ECS/Component/Component.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"

#include <imgui.h>

namespace neo {

	struct RotationComponent : public Component {
		RotationComponent(glm::vec3 s)
			: mSpeed(s)
		{}
		glm::vec3 mSpeed;

		virtual std::string getName() const override {
			return "RotationComponent";
		}

		virtual void imGuiEditor() override {
			ImGui::SliderFloat3("Speed", &mSpeed[0], -5.f, 5.f);
		}

	};
}