#pragma once

#include "ShaderComponent.hpp"

#include "Loader/Library.hpp"

namespace neo {
	// TODO - there's no wireframe rendering system
	struct WireframeShaderComponent : public ShaderComponent {
        glm::vec3 mColor = glm::vec3(1.f);
		WireframeShaderComponent() : ShaderComponent({
			Library::createShaderSource("WireframeShader", SourceShader::ShaderCode{
				// TODO - memory leak
				{ ShaderStage::VERTEX, Loader::loadFileString("model.vert")},
				{ ShaderStage::FRAGMENT, R"(
					uniform vec3 wireColor;
					out vec4 color;
					void main() {
					    color = vec4(wireColor, 1.0);
					})"
				}
			})
		})
		{}

		virtual void imGuiEditor() override {
            ImGui::ColorEdit3("Color", &mColor[0]);
        };

		virtual std::string getName() const override {
			return "WireframeShaderComponent";
		}
	};
}