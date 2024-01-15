#pragma once

#include "ECS/Component/Component.hpp"

#include <imgui.h>

namespace neo {
    class NewTexture; 
	struct MaterialComponent : public Component {
        glm::vec3 mAmbient;

        NewTexture* mDiffuseMap = nullptr;
        glm::vec3 mDiffuse;

        NewTexture* mSpecularMap = nullptr;
        glm::vec3 mSpecular;

        NewTexture* mNormalMap = nullptr;

        NewTexture* mAlphaMap = nullptr;

        float mShininess;

        // Unused..for now
        glm::vec3 mTransmittance;
        glm::vec3 mEmission;
        float mIOR;      // index of refraction
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