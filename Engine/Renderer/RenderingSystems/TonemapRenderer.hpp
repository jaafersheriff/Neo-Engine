#pragma once

#include "Renderer/GLObjects/SourceShader.hpp"
#include "Renderer/GLObjects/ResolvedShaderInstance.hpp"

#include "ResourceManager/ResourceManagers.hpp"

namespace neo {

	static void tonemap(const ResourceManagers& resourceManagers, glm::uvec2 dimension, TextureHandle inputTextureHandle) {
		TRACY_GPU();

		if (!resourceManagers.mTextureManager.isValid(inputTextureHandle)) {
			return;
		}

		auto tonemapShaderHandle = resourceManagers.mShaderManager.asyncLoad("Tonemap Shader", SourceShader::ConstructionArgs{
			{ types::shader::Stage::Vertex, "quad.vert"},
			{ types::shader::Stage::Fragment, "tonemap.frag" }
		});
		if (!resourceManagers.mShaderManager.isValid(tonemapShaderHandle)) {
			return;
		}

		glViewport(0, 0, dimension.x, dimension.y);

		auto& resolvedShader = resourceManagers.mShaderManager.resolveDefines(tonemapShaderHandle, {});
		resolvedShader.bind();

		auto& inputTexture = resourceManagers.mTextureManager.resolve(inputTextureHandle);
		resolvedShader.bindTexture("inputTexture", inputTexture);

		glDisable(GL_DEPTH_TEST);
		resourceManagers.mMeshManager.resolve("quad").draw();
		glEnable(GL_DEPTH_TEST);
	}
}