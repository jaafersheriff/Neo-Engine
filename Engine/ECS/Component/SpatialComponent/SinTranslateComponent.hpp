#pragma once

#include "ECS/Component/Component.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"

namespace neo {

	struct SinTranslateComponent : public Component {
		glm::vec3 mOffset;
		glm::vec3 mBasePosition;

		SinTranslateComponent(glm::vec3 offset, glm::vec3 base) 
			: mOffset(offset),
			mBasePosition(base + offset * util::genRandom())
		{
		}

		virtual std::string getName() const override {
			return "SinTranslateComponent";
		}

		virtual void imGuiEditor() override {
			ImGui::SliderFloat3("Offset", &mOffset[0], -10.f, 10.f);
			ImGui::SliderFloat3("Base position", &mBasePosition[0], -100.f, 100.f);
		}

	};
}
