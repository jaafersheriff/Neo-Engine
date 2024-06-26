#pragma once

#include "Renderer/GLObjects/SourceShader.hpp"
#include "Renderer/GLObjects/ResolvedShaderInstance.hpp"

#include "ResourceManager/ResourceManagers.hpp"

namespace neo {

	static FramebufferHandle tonemap(const ResourceManagers& resourceManagers, glm::uvec2 dimension, TextureHandle inputTextureHandle) {
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

		tonemapTarget.clear(glm::vec4(0.f, 0.f, 0.f, 1.f), types::framebuffer::AttachmentBit::Color);

		auto tonemapShaderHandle = resourceManagers.mShaderManager.asyncLoad("Tonemap Shader", SourceShader::ConstructionArgs{
			{ types::shader::Stage::Vertex, "quad.vert"},
			{ types::shader::Stage::Fragment, "tonemap.frag" }
		});
		if (!resourceManagers.mShaderManager.isValid(tonemapShaderHandle)) {
			return NEO_INVALID_HANDLE;
		}

		glViewport(0, 0, dimension.x, dimension.y);

		auto& resolvedShader = resourceManagers.mShaderManager.resolveDefines(tonemapShaderHandle, {});
		resolvedShader.bind();

		auto& inputTexture = resourceManagers.mTextureManager.resolve(inputTextureHandle);
		resolvedShader.bindTexture("inputTexture", inputTexture);

		glDisable(GL_DEPTH_TEST);
		resourceManagers.mMeshManager.resolve("quad").draw();
		glEnable(GL_DEPTH_TEST);

		return tonemapTargetHandle;
	}
}