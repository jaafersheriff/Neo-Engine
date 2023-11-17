#pragma once

#include "ShaderComponent.hpp"

#include "Loader/Library.hpp"

namespace neo {
	struct PhongShaderComponent : public ShaderComponent {
		PhongShaderComponent() : ShaderComponent({
			Library::createSourceShader("PhongShader", SourceShader::ConstructionArgs{
				{ ShaderStage::VERTEX, "phong.vert"},
				{ ShaderStage::FRAGMENT, "phong.frag" }
			})
		})
		{}

		virtual std::string getName() const override {
			return "PhongShaderComponent";
		}
	};
}