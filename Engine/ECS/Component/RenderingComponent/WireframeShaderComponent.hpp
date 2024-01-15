#pragma once

#include "ShaderComponent.hpp"

#include "Loader/Library.hpp"

#include <imgui.h>

namespace neo {
	struct WireframeShaderComponent : public ShaderComponent {
		glm::vec3 mColor = glm::vec3(1.f);

		WireframeShaderComponent(glm::vec3 color = glm::vec3(1.f)) 
			: ShaderComponent({
				Library::createSourceShader("WireframeShader", SourceShader::ConstructionArgs{
					{ ShaderStage::VERTEX, "model.vert"},
					{ ShaderStage::FRAGMENT, "wireframe.frag" }
				})
			})
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