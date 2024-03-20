#pragma once

#include "ECS/Component/Component.hpp"

#include "ResourceManager/TextureResourceManager.hpp"

#include <imgui.h>

namespace neo {
	struct MaterialComponent: public Component {
		bool mDoubleSided = false;

		glm::vec4 mAlbedoColor = glm::vec4(0.f, 0.f, 0.f, 1.f);
		TextureHandle mAlbedoMap;

		float mMetallic = 0.f;
		float mRoughness = 0.7f;
		TextureHandle mMetallicRoughnessMap;

		glm::vec3 mEmissiveFactor = glm::vec3(0.f);
		TextureHandle mEmissiveMap;

		TextureHandle mNormalMap;
		TextureHandle mOcclusionMap;

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