#pragma once

#include "Renderer/GLObjects/SourceShader.hpp"
#include "Renderer/GLObjects/ResolvedShaderInstance.hpp"

#include "Loader/Library.hpp"
#include "Loader/Loader.hpp"

namespace neo {

	static void drawFXAA(glm::uvec2 dimension, Texture& inputTexture, const MeshManager& meshManager) {
		TRACY_GPU();

		auto* fxaaShader = Library::createSourceShader("FXAAShader", SourceShader::ConstructionArgs{
			{ ShaderStage::VERTEX, "quad.vert"},
			{ ShaderStage::FRAGMENT, "fxaa.frag" }
		});

		glViewport(0, 0, dimension.x, dimension.y);

		auto& resolvedShader = fxaaShader->getResolvedInstance({});
		resolvedShader.bind();
		resolvedShader.bindUniform("frameSize", glm::vec2(inputTexture.mWidth, inputTexture.mHeight));
		resolvedShader.bindTexture("inputTexture", inputTexture);

		glDisable(GL_DEPTH_TEST);
		meshManager.get("quad").draw();
		glEnable(GL_DEPTH_TEST);
	}
}