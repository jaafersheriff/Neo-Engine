#pragma once

#include "Renderer/GLObjects/SourceShader.hpp"
#include "Renderer/GLObjects/ResolvedShaderInstance.hpp"

#include "ResourceManager/ResourceManagers.hpp"

namespace neo {

	static FramebufferHandle bloom(const ResourceManagers& resourceManagers, const glm::uvec2 dimension, const TextureHandle inputTextureHandle, const float radius = 0.005, const int downSampleSteps = 6) {
		TRACY_GPU();

		NEO_ASSERT(downSampleSteps > 0, "Gotta bloom with something");

		if (!resourceManagers.mTextureManager.isValid(inputTextureHandle)) {
			return NEO_INVALID_HANDLE;
		}
		const auto& inputTexture = resourceManagers.mTextureManager.resolve(inputTextureHandle);
		const auto& quadMesh = resourceManagers.mMeshManager.resolve("quad");
		glm::uvec2 baseDimension = dimension / glm::uvec2(2);

		// Create textures and targets
		std::vector<FramebufferHandle> bloomTargets;
		for (int i = 0; i < downSampleSteps; i++) {
			bloomTargets.push_back(resourceManagers.mFramebufferManager.asyncLoad(
				HashedString((std::string("BloomTarget") + std::to_string(i)).c_str()),
				FramebufferBuilder{}
					.setSize(glm::uvec2(baseDimension.x >> i, baseDimension.y >> i))
					.attach(TextureFormat{
						types::texture::Target::Texture2D,
						types::texture::InternalFormats::RGB16_F
					}
				),
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
			MakeDefine(MIP_0);
			ShaderDefines Mip0Defines;
			Mip0Defines.set(MIP_0);

			glDisable(GL_DEPTH_TEST);
			for (int i = 0; i < downSampleSteps; i++) {
				if (resourceManagers.mFramebufferManager.isValid(bloomTargets[i])) {
					auto& bloomDown = resourceManagers.mFramebufferManager.resolve(bloomTargets[i]);
					bloomDown.bind();
					bloomDown.clear(glm::vec4(0.f, 0.f, 0.f, 1.f), types::framebuffer::AttachmentBit::Color);

					auto& resolvedShader = resourceManagers.mShaderManager.resolveDefines(bloomDownShaderHandle, i == 0 ? Mip0Defines : ShaderDefines{});
					resolvedShader.bind();
					if (i == 0) {
						resolvedShader.bindTexture("inputTexture", inputTexture);
					}

					glm::uvec2 mipDimension(baseDimension.x >> i, baseDimension.y >> i);
					glViewport(0, 0, mipDimension.x, mipDimension.y);
					resolvedShader.bindUniform("texelSize", glm::vec2(1.f / glm::vec2(mipDimension)));
					quadMesh.draw();
					resolvedShader.bindTexture("inputTexture", resourceManagers.mTextureManager.resolve(bloomDown.mTextures[0]));
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
					resolvedShader.bindTexture("inputTexture", resourceManagers.mTextureManager.resolve(resourceManagers.mFramebufferManager.resolve(bloomTargets[i]).mTextures[0]));
					auto& bloomUp = resourceManagers.mFramebufferManager.resolve(bloomTargets[i - 1]);
					bloomUp.bind();

					glm::uvec2 mipDimension(baseDimension.x >> (i - 1), baseDimension.y >> (i - 1));
					glViewport(0, 0, mipDimension.x, mipDimension.y);
					quadMesh.draw();
				}
			}
			glDisable(GL_BLEND);

			// Create a new full-res render target
			auto bloomOutputHandle = resourceManagers.mFramebufferManager.asyncLoad(
				HashedString("BloomOutput"),
				FramebufferBuilder{}
				.setSize(glm::uvec2(inputTexture.mWidth, inputTexture.mHeight))
				.attach({ types::texture::Target::Texture2D, types::texture::InternalFormats::RGB16_F }),
				resourceManagers.mTextureManager
			);
			if (resourceManagers.mFramebufferManager.isValid(bloomOutputHandle)) {
				auto bloomOutput = resourceManagers.mFramebufferManager.resolve(bloomOutputHandle);

				auto bloomMixShaderHandle = resourceManagers.mShaderManager.asyncLoad("BloomMix Shader", SourceShader::ConstructionArgs{
					{ types::shader::Stage::Vertex, "quad.vert"},
					{ types::shader::Stage::Fragment, "bloomMix.frag" }
					});
				if (resourceManagers.mShaderManager.isValid(bloomMixShaderHandle)) {
					bloomOutput.bind();

					auto bloomMixShader = resourceManagers.mShaderManager.resolveDefines(bloomMixShaderHandle, {});

					bloomMixShader.bind();
					bloomMixShader.bindTexture("bloomResults", resourceManagers.mTextureManager.resolve(resourceManagers.mFramebufferManager.resolve(bloomTargets[0]).mTextures[0]));
					bloomMixShader.bindTexture("hdrColor", inputTexture);
					glViewport(0, 0, dimension.x, dimension.y);
					quadMesh.draw();
					glEnable(GL_DEPTH_TEST);
				}
			}

			return bloomOutputHandle;
		}
	}
}