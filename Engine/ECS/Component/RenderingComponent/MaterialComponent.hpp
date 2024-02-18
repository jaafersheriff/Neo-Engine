#pragma once

#include "ECS/Component/Component.hpp"

#include <imgui.h>

namespace neo {
	class Texture; 

	struct MaterialComponent: public Component {
		glm::vec4 mAlbedoColor = glm::vec4(0.f, 0.f, 0.f, 1.f);
		Texture* mAlbedoMap = nullptr;

		float mMetallic = 0.f;
		float mRoughness = 0.7f;
		Texture* mMetallicRoughnessMap = nullptr;

		glm::vec3 mEmissiveFactor = glm::vec3(0.f);
		Texture* mEmissiveMap = nullptr;

		Texture* mNormalMap = nullptr;
		Texture* mOcclusionMap = nullptr;

		virtual std::string getName() const override {
			return "MaterialComponent";
		}

		virtual void imGuiEditor() override {
			ImGui::ColorEdit3("Albedo", &mAlbedoColor[0]);
			ImGui::SliderFloat("Metallness", &mMetallic, 0.f, 1.f); // This should be a bool?
			ImGui::SliderFloat("Roughness", &mRoughness, 0.f, 1.f);
			if (mEmissiveMap) {
				ImGui::ColorEdit3("Emissive Factor", &mEmissiveFactor[0]);
			}
		};
	};

}