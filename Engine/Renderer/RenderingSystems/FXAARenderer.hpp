#pragma once

#include "Renderer/GLObjects/SourceShader.hpp"
#include "Renderer/GLObjects/ResolvedShaderInstance.hpp"

#include "ResourceManager/ResourceManagers.hpp"

namespace neo {

	inline void drawFXAA(const ResourceManagers& resourceManagers, TextureHandle inputTextureHandle) {
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

		auto& resolvedShader = resourceManagers.mShaderManager.resolveDefines(fxaaShaderHandle, {});
		resolvedShader.bind();

		auto& inputTexture = resourceManagers.mTextureManager.resolve(inputTextureHandle);
		resolvedShader.bindUniform("frameSize", glm::vec2(inputTexture.mWidth, inputTexture.mHeight));
		resolvedShader.bindTexture("inputTexture", inputTexture);

		resourceManagers.mMeshManager.resolve("quad").draw();
	}
}