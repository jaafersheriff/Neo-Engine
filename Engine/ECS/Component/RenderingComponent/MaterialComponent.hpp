#pragma once

#include "ECS/Component/Component.hpp"

#include <imgui.h>

namespace neo {
	class Texture; 
	struct MaterialComponent : public Component {
		glm::vec3 mAmbient = glm::vec3(0.2f);

		Texture* mDiffuseMap = nullptr;
		glm::vec3 mDiffuse = glm::vec3(0.5f);

		Texture* mSpecularMap = nullptr;
		glm::vec3 mSpecular = glm::vec3(0.f);

		Texture* mNormalMap = nullptr;

		Texture* mAlphaMap = nullptr;

		float mShininess = 0.f;

		// Unused..for now
		glm::vec3 mTransmittance;
		glm::vec3 mEmission;
		float mIOR;	  // index of refraction
		float mDissolve; // 1 == opaque; 0 == fully transparent

		virtual std::string getName() const override {
			return "MaterialComponent";
		}

		virtual void imGuiEditor() override {
			ImGui::ColorEdit3("Ambient", &mAmbient[0]);
			ImGui::ColorEdit3("Diffuse", &mDiffuse[0]);
			ImGui::ColorEdit3("Specular", &mSpecular[0]);
			ImGui::SliderFloat("Shine", &mShininess, 0.f, 50.f);
		};
	};
}