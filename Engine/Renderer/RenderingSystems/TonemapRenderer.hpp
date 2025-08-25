#pragma once

#include "Renderer/GLObjects/SourceShader.hpp"
#include "Renderer/GLObjects/ResolvedShaderInstance.hpp"

#include "ResourceManager/ResourceManagers.hpp"

namespace neo {

	inline FramebufferHandle tonemap(const ResourceManagers& resourceManagers, glm::uvec2 dimension, TextureHandle inputTextureHandle, TextureHandle averageLuminance = NEO_INVALID_HANDLE) {
		TRACY_GPU();

		if (!resourceManagers.mTextureManager.isValid(inputTextureHandle)) {
			return NEO_INVALID_HANDLE;
		}
		auto tonemapTargetHandle = resourceManagers.mFramebufferManager.asyncLoad(
			"Tonemapped",
			FramebufferBuilder{}
				.setSize(dimension)
				.attach(TextureFormat{ types::texture::Target::Texture2D, types::texture::InternalFormats::RGB8_UNORM }),
			resourceManagers.mTextureManager
		);
		if (!resourceManagers.mFramebufferManager.isValid(tonemapTargetHandle)) {
			return NEO_INVALID_HANDLE;
		}
		auto& tonemapTarget = resourceManagers.mFramebufferManager.resolve(tonemapTargetHandle);
		tonemapTarget.bind();
		glViewport(0, 0, dimension.x, dimension.y);

		tonemapTarget.clear(glm::vec4(0.f, 0.f, 0.f, 1.f), types::framebuffer::AttachmentBit::Color);

		auto tonemapShaderHandle = resourceManagers.mShaderManager.asyncLoad("Tonemap Shader", SourceShader::ConstructionArgs{
			{ types::shader::Stage::Vertex, "quad.vert"},
			{ types::shader::Stage::Fragment, "tonemap.frag" }
		});
		if (!resourceManagers.mShaderManager.isValid(tonemapShaderHandle)) {
			return NEO_INVALID_HANDLE;
		}

		ShaderDefines defines;
		MakeDefine(AUTO_EXPOSURE);
		if (resourceManagers.mTextureManager.isValid(averageLuminance)) {
			defines.set(AUTO_EXPOSURE);
		}

		auto& resolvedShader = resourceManagers.mShaderManager.resolveDefines(tonemapShaderHandle, defines);
		resolvedShader.bind();

		auto& inputTexture = resourceManagers.mTextureManager.resolve(inputTextureHandle);
		resolvedShader.bindTexture("inputTexture", inputTexture);
		if (resourceManagers.mTextureManager.isValid(averageLuminance)) {
			resolvedShader.bindTexture("averageLuminance", resourceManagers.mTextureManager.resolve(averageLuminance));
		}

		glDisable(GL_DEPTH_TEST);
		int oldPolygonMode;
		glGetIntegerv(GL_POLYGON_MODE, &oldPolygonMode);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	
		resourceManagers.mMeshManager.resolve("quad").draw();
		glEnable(GL_DEPTH_TEST);
		glPolygonMode(GL_FRONT_AND_BACK, oldPolygonMode);

		return tonemapTargetHandle;
	}
}