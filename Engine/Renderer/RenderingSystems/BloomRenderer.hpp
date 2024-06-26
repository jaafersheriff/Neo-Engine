#pragma once

#include "Renderer/GLObjects/SourceShader.hpp"
#include "Renderer/GLObjects/ResolvedShaderInstance.hpp"

#include "ResourceManager/ResourceManagers.hpp"

namespace neo {

	static FramebufferHandle bloom(const ResourceManagers& resourceManagers, glm::uvec2 dimension, TextureHandle inputTextureHandle, int downSampleSteps = 6) {
		TRACY_GPU();

		if (!resourceManagers.mTextureManager.isValid(inputTextureHandle)) {
			return NEO_INVALID_HANDLE;
		}

		auto bloomDownShaderHandle = resourceManagers.mShaderManager.asyncLoad("BloomDown Shader", SourceShader::ConstructionArgs{
			{ types::shader::Stage::Vertex, "quad.vert"},
			{ types::shader::Stage::Fragment, "bloomDown.frag" }
		});
		if (!resourceManagers.mShaderManager.isValid(bloomDownShaderHandle)) {
			return NEO_INVALID_HANDLE;
		}
		auto& resolvedShader = resourceManagers.mShaderManager.resolveDefines(bloomDownShaderHandle, {});
		resolvedShader.bind();

		auto& inputTexture = resourceManagers.mTextureManager.resolve(inputTextureHandle);
		resolvedShader.bindTexture("inputTexture", inputTexture);

		glDisable(GL_DEPTH_TEST);
		std::vector<TextureHandle> downSampleTextures;
		for (int i = 0; i < downSampleSteps; i++) {
			downSampleTextures.push_back(resourceManagers.mTextureManager.asyncLoad(
				"TODO", 
				TextureBuilder{}
					.setDimension(glm::u16vec3(dimension.x >> i, dimension.y >> i, 0))
					.setFormat(TextureFormat{
						types::texture::Target::Texture2D,
						types::texture::InternalFormats::RGB16_F
					})
			));
		}

		for (int i = 0; i < downSampleSteps; i++) {
			auto bloomDownHandle = resourceManagers.mFramebufferManager.asyncLoad(
				"TODO",
				FramebufferExternal{
					{downSampleTextures[i]}
				},
				resourceManagers.mTextureManager
			);

			if (resourceManagers.mFramebufferManager.isValid(bloomDownHandle)) {
				auto& bloomDown = resourceManagers.mFramebufferManager.resolve(bloomDownHandle);
				bloomDown.bind();
				bloomDown.clear(glm::vec4(0.f, 0.f, 0.f, 1.f), types::framebuffer::AttachmentBit::Color);

				glm::uvec2 mipDimension(dimension.x >> i, dimension.y >> i);

				glViewport(0, 0, mipDimension.x, mipDimension.y);

				resolvedShader.bindUniform("texelSize", glm::vec2(1.f / glm::vec2(mipDimension)));
				resourceManagers.mMeshManager.resolve("quad").draw();
			}
		}

		glEnable(GL_DEPTH_TEST);
		return NEO_INVALID_HANDLE; // TODOs
	}
}