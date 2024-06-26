#pragma once

#include "Renderer/GLObjects/SourceShader.hpp"
#include "Renderer/GLObjects/ResolvedShaderInstance.hpp"

#include "ResourceManager/ResourceManagers.hpp"

namespace neo {

	static FramebufferHandle bloom(const ResourceManagers& resourceManagers, glm::uvec2 dimension, TextureHandle inputTextureHandle, float radius, int downSampleSteps = 6) {
		TRACY_GPU();

		NEO_ASSERT(downSampleSteps > 0, "Gotta bloom with something");

		if (!resourceManagers.mTextureManager.isValid(inputTextureHandle)) {
			return NEO_INVALID_HANDLE;
		}

		// Create textures and targets
		std::vector<TextureHandle> bloomTextures;
		std::vector<FramebufferHandle> bloomTargets;
		for (int i = 0; i < downSampleSteps; i++) {
			bloomTextures.push_back(resourceManagers.mTextureManager.asyncLoad(
				HashedString((std::string("BloomTex") + std::to_string(i)).c_str()),
				TextureBuilder{}
				.setDimension(glm::u16vec3(dimension.x >> i, dimension.y >> i, 0))
				.setFormat(TextureFormat{
					types::texture::Target::Texture2D,
					types::texture::InternalFormats::RGB16_F
					})
			));

			bloomTargets.push_back(resourceManagers.mFramebufferManager.asyncLoad(
				HashedString((std::string("BloomTarget") + std::to_string(i)).c_str()),
				FramebufferExternal{
					{bloomTextures[i]}
				},
				resourceManagers.mTextureManager
			));
		}

		// Down sample
		{
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
			for (int i = 0; i < downSampleSteps; i++) {
				if (resourceManagers.mFramebufferManager.isValid(bloomTargets[i])) {
					auto& bloomDown = resourceManagers.mFramebufferManager.resolve(bloomTargets[i]);
					bloomDown.bind();
					bloomDown.clear(glm::vec4(0.f, 0.f, 0.f, 1.f), types::framebuffer::AttachmentBit::Color);

					glm::uvec2 mipDimension(dimension.x >> i, dimension.y >> i);
					glViewport(0, 0, mipDimension.x, mipDimension.y);
					resolvedShader.bindUniform("texelSize", glm::vec2(1.f / glm::vec2(mipDimension)));
					resourceManagers.mMeshManager.resolve("quad").draw();
					resolvedShader.bindTexture("inputTexture", resourceManagers.mTextureManager.resolve(bloomTextures[i]));
				}
			}
		}

		// Up sample
		{
			auto bloomUpShaderHandle = resourceManagers.mShaderManager.asyncLoad("BloomUp Shader", SourceShader::ConstructionArgs{
				{ types::shader::Stage::Vertex, "quad.vert"},
				{ types::shader::Stage::Fragment, "bloomUp.frag" }
				});
			if (!resourceManagers.mShaderManager.isValid(bloomUpShaderHandle)) {
				return NEO_INVALID_HANDLE;
			}
			auto& resolvedShader = resourceManagers.mShaderManager.resolveDefines(bloomUpShaderHandle, {});
			resolvedShader.bind();

			resolvedShader.bindUniform("filterRadius", radius);

			glEnable(GL_BLEND);
			glBlendFunc(GL_ONE, GL_ONE);
			glBlendEquation(GL_FUNC_ADD);
			for (int i = downSampleSteps - 1; i > 0; i--) {
				if (resourceManagers.mFramebufferManager.isValid(bloomTargets[i])) {
					resolvedShader.bindTexture("inputTexture", resourceManagers.mTextureManager.resolve(bloomTextures[i]));
					auto& bloomUp = resourceManagers.mFramebufferManager.resolve(bloomTargets[i-1]);
					bloomUp.bind();

					glm::uvec2 mipDimension(dimension.x >> (i-1), dimension.y >> (i-1));
					glViewport(0, 0, mipDimension.x, mipDimension.y);
					resourceManagers.mMeshManager.resolve("quad").draw();
				}
			}
			glDisable(GL_BLEND);
			glEnable(GL_DEPTH_TEST);
		}

		if (resourceManagers.mFramebufferManager.isValid(bloomTargets[0])) {
			return bloomTargets[0];
		}
		return NEO_INVALID_HANDLE;
	}
}