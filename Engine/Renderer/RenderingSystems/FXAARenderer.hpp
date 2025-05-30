#pragma once

#include "Renderer/GLObjects/SourceShader.hpp"
#include "Renderer/GLObjects/ResolvedShaderInstance.hpp"

#include "ResourceManager/ResourceManagers.hpp"

namespace neo {

	inline void drawFXAA(const ResourceManagers& resourceManagers, glm::uvec2 dimension, TextureHandle inputTextureHandle) {
		TRACY_GPU();

		if (!resourceManagers.mTextureManager.isValid(inputTextureHandle)) {
			return;
		}

		auto fxaaShaderHandle = resourceManagers.mShaderManager.asyncLoad("FXAAShader", SourceShader::ConstructionArgs{
			{ types::shader::Stage::Vertex, "quad.vert"},
			{ types::shader::Stage::Fragment, "fxaa.frag" }
		});
		if (!resourceManagers.mShaderManager.isValid(fxaaShaderHandle)) {
			return;
		}

		glViewport(0, 0, dimension.x, dimension.y);

		auto& resolvedShader = resourceManagers.mShaderManager.resolveDefines(fxaaShaderHandle, {});
		resolvedShader.bind();

		auto& inputTexture = resourceManagers.mTextureManager.resolve(inputTextureHandle);
		resolvedShader.bindUniform("frameSize", glm::vec2(inputTexture.mWidth, inputTexture.mHeight));
		resolvedShader.bindTexture("inputTexture", inputTexture);

		glDisable(GL_DEPTH_TEST);
		int oldPolygonMode;
		glGetIntegerv(GL_POLYGON_MODE, &oldPolygonMode);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		resourceManagers.mMeshManager.resolve("quad").draw();
		glEnable(GL_DEPTH_TEST);
		glPolygonMode(GL_FRONT_AND_BACK, oldPolygonMode);
	}
}