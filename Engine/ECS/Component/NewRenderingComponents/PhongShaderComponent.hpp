#pragma once

#include "ShaderComponent.hpp"

namespace neo {
	struct PhongShaderComponent : public ShaderComponent {
		PhongShaderComponent() : ShaderComponent({
			Library::createShaderSource("PhongShader", {
				{ NewShader::ShaderStage::VERTEX, "phong.vert"},
				{ NewShader::ShaderStage::FRAGMENT, "phong.frag" }
			})
		})
		{}

		virtual std::string getName() const override {
			return "PhongShaderComponent";
		}
	};
}