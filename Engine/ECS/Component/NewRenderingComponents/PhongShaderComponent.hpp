#pragma once

#include "ShaderComponent.hpp"

#include "Loader/Library.hpp"

namespace neo {
	struct PhongShaderComponent : public ShaderComponent {
		PhongShaderComponent() : ShaderComponent({
			Library::createShaderSource("PhongShader", {
				{ ShaderStage::VERTEX, Loader::loadFileString("phong.vert")},
				{ ShaderStage::FRAGMENT, Loader::loadFileString("phong.frag") }
			})
		})
		{}

		virtual std::string getName() const override {
			return "PhongShaderComponent";
		}
	};
}