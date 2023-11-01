#pragma once

#include "ShaderComponent.hpp"

namespace neo {
	struct PhongShaderComponent : public ShaderComponent {
		PhongShaderComponent() : ShaderComponent({
			Library::createShaderSource("PhongShader", {
				{ NewShader::ShaderStage::VERTEX, Library::loadShaderFile("phong.vert")},
				{ NewShader::ShaderStage::FRAGMENT, Library::loadShaderFile("phong.frag") }
			})
		})
		{}

		virtual std::string getName() const override {
			return "PhongShaderComponent";
		}
	};
}