#pragma once

#include "Renderer/GLObjects/SourceShader.hpp"
#include "Renderer/GLObjects/ResolvedShaderInstance.hpp"

#include "Loader/Library.hpp"
#include "Loader/Loader.hpp"

#include "ResourceManager/ResourceManagers.hpp"

namespace neo {

	static void drawFXAA(const ResourceManagers& resourceManagers, glm::uvec2 dimension, Texture& inputTexture) {
		TRACY_GPU();

		auto fxaaShaderHandle = resourceManagers.mShaderManager.asyncLoad("FXAAShader", SourceShader::ConstructionArgs{
			{ ShaderStage::VERTEX, "quad.vert"},
			{ ShaderStage::FRAGMENT, "fxaa.frag" }
		});
		if (!resourceManagers.mShaderManager.isValid(fxaaShaderHandle)) {
			return;
		}

		glViewport(0, 0, dimension.x, dimension.y);

		auto& resolvedShader = resourceManagers.mShaderManager.get(fxaaShaderHandle, {});
		resolvedShader.bind();
		resolvedShader.bindUniform("frameSize", glm::vec2(inputTexture.mWidth, inputTexture.mHeight));
		resolvedShader.bindTexture("inputTexture", inputTexture);

		glDisable(GL_DEPTH_TEST);
		resourceManagers.mMeshManager.get("quad").draw();
		glEnable(GL_DEPTH_TEST);
	}
}