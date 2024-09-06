#pragma once

#include "ECS/Component/Component.hpp"

#include "ResourceManager/TextureManager.hpp"

#include <ext/imgui_incl.hpp>

namespace neo {
	START_COMPONENT(MaterialComponent);
		glm::vec4 mAlbedoColor = glm::vec4(0.f, 0.f, 0.f, 1.f);
		TextureHandle mAlbedoMap;

		float mMetallic = 0.f;
		float mRoughness = 0.7f;
		TextureHandle mMetallicRoughnessMap;

		glm::vec3 mEmissiveFactor = glm::vec3(0.f);
		TextureHandle mEmissiveMap;

		TextureHandle mNormalMap;
		TextureHandle mOcclusionMap;

		virtual void imGuiEditor() override {
			ImGui::ColorEdit4("Albedo", &mAlbedoColor[0]);
			ImGui::SliderFloat("Metallness", &mMetallic, 0.f, 1.f); // This should be a bool?
			ImGui::SliderFloat("Roughness", &mRoughness, 0.f, 1.f);
			if (ImGui::ColorEdit3("Emissive Factor", &mEmissiveFactor[0], ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_NoAlpha)) {
				mEmissiveFactor = glm::max(mEmissiveFactor, glm::vec3(0.f));
			}
		};
	END_COMPONENT();
}
