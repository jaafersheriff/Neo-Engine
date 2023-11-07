#pragma once

#include "ShaderComponent.hpp"

#include "Loader/Library.hpp"

namespace neo {

    struct ShadowCasterShaderComponent : public ShaderComponent {
		ShadowCasterShaderComponent() : ShaderComponent({
			Library::createShaderSource("ShadowCasterShader", NewShader::ConstructionArgs{
				{ ShaderStage::VERTEX, "model.vert"},
				{ ShaderStage::FRAGMENT, "depth.frag" }
			})
		})
		{}

        virtual std::string getName() const override {
            return "ShadowCasterShaderComponent";
        }

    };
}