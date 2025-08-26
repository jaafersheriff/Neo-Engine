#pragma once

#include "Renderer/GLObjects/SourceShader.hpp"
#include "Renderer/GLObjects/ResolvedShaderInstance.hpp"

#include "ResourceManager/ResourceManagers.hpp"

namespace neo {

	struct BloomParameters {
		float mRadius = 0.005f; 
		int mDownSampleSteps = 6;
		float mLuminanceThreshold = 1.f;

		void imguiEditor() {
			if (ImGui::TreeNodeEx("Bloom Parameters", ImGuiTreeNodeFlags_DefaultOpen)) {
				ImGui::SliderFloat("Radius", &mRadius, 0.001f, 0.010f);
				ImGui::SliderInt("Down Samples", &mDownSampleSteps, 1, 10);
				ImGui::SliderFloat("Luminance Threshold", &mLuminanceThreshold, 0.f, 100000.f);
				ImGui::TreePop();
			}
		}
	};

	inline FramebufferHandle bloom(RenderPasses& renderPasses, const ResourceManagers& resourceManagers, const glm::uvec2 dimension, const TextureHandle inputTextureHandle, const BloomParameters& parameters) {
		TRACY_GPU();

		NEO_ASSERT(parameters.mDownSampleSteps > 0, "Gotta bloom with something");

		if (!resourceManagers.mTextureManager.isValid(inputTextureHandle)) {
			return NEO_INVALID_HANDLE;
		}

		// Create textures and targets
		glm::uvec2 baseDimension = dimension / glm::uvec2(2);
		std::vector<TextureHandle> bloomTextures;
		std::vector<FramebufferHandle> bloomTargets;
		std::string textureName = "BloomTextureN";
		std::string targetName = "BloomTargetN";
		for (int i = 0; i < parameters.mDownSampleSteps; i++) {
			textureName.back() = static_cast<char>(static_cast<int>('0') + i);
			targetName.back() = static_cast<char>(static_cast<int>('0') + i);

			bloomTextures.push_back(resourceManagers.mTextureManager.asyncLoad(
				HashedString(targetName.c_str()),
				TextureBuilder{}
				.setDimension(glm::u16vec3(baseDimension.x >> i, baseDimension.y >> i, 0))
				.setFormat(TextureFormat{
					types::texture::Target::Texture2D,
					types::texture::InternalFormats::RGB16_F
					}
				)
			));
			bloomTargets.push_back(resourceManagers.mFramebufferManager.asyncLoad(
				HashedString(targetName.c_str()),
				FramebufferExternalAttachments{
					FramebufferAttachment{bloomTextures[i]}
				},
				resourceManagers.mTextureManager
			));
		}

		// Down sample
		{
			MakeDefine(MIP_0);
			ShaderDefines Mip0Defines;
			Mip0Defines.set(MIP_0);

			for (int i = 0; i < parameters.mDownSampleSteps; i++) {
				renderPasses.clear(bloomTargets[i], types::framebuffer::AttachmentBit::Color, glm::vec4(0.f, 0.f, 0.f, 1.f));
				glm::uvec2 mipDimension(baseDimension.x >> i, baseDimension.y >> i);
				renderPasses.renderPass(bloomTargets[i], mipDimension, [i, Mip0Defines, bloomTextures, mipDimension, inputTextureHandle, parameters](const ResourceManagers& resourceManagers, const ECS& ecs) {
					glDisable(GL_DEPTH_TEST);
					int oldPolygonMode;
					glGetIntegerv(GL_POLYGON_MODE, &oldPolygonMode);
					glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

					auto bloomDownShaderHandle = resourceManagers.mShaderManager.asyncLoad("BloomDown Shader", SourceShader::ConstructionArgs{
						{ types::shader::Stage::Vertex, "quad.vert"},
						{ types::shader::Stage::Fragment, "bloomDown.frag" }
						});
					if (!resourceManagers.mShaderManager.isValid(bloomDownShaderHandle)) {
						return;
					}
					auto& resolvedShader = resourceManagers.mShaderManager.resolveDefines(bloomDownShaderHandle, i == 0 ? Mip0Defines : ShaderDefines{});
					resolvedShader.bind();
					if (i == 0) {
						const auto& inputTexture = resourceManagers.mTextureManager.resolve(inputTextureHandle);
						resolvedShader.bindTexture("inputTexture", inputTexture);
						resolvedShader.bindUniform("threshold", parameters.mLuminanceThreshold);
					}
					else {
						resolvedShader.bindTexture("inputTexture", resourceManagers.mTextureManager.resolve(bloomTextures[i - 1]));
					}

					resolvedShader.bindUniform("texelSize", glm::vec2(1.f / glm::vec2(mipDimension)));
					const auto& quadMesh = resourceManagers.mMeshManager.resolve("quad");
					quadMesh.draw();
					glEnable(GL_DEPTH_TEST);
					glPolygonMode(GL_FRONT_AND_BACK, oldPolygonMode);
				});
			}
		}

		// Up sample
		{

			for (int i = parameters.mDownSampleSteps - 1; i > 0; i--) {
				renderPasses.renderPass(bloomTargets[i - 1], glm::uvec2(baseDimension.x >> (i - 1), baseDimension.y >> (i - 1)), [parameters, i, bloomTextures](const ResourceManagers& resourceManagers, const ECS& ecs) {

					glDisable(GL_DEPTH_TEST);
					int oldPolygonMode;
					glGetIntegerv(GL_POLYGON_MODE, &oldPolygonMode);
					glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

					glEnable(GL_BLEND);
					glBlendFunc(GL_ONE, GL_ONE);
					glBlendEquation(GL_FUNC_ADD);

					auto bloomUpShaderHandle = resourceManagers.mShaderManager.asyncLoad("BloomUp Shader", SourceShader::ConstructionArgs{
						{ types::shader::Stage::Vertex, "quad.vert"},
						{ types::shader::Stage::Fragment, "bloomUp.frag" }
						});
					if (!resourceManagers.mShaderManager.isValid(bloomUpShaderHandle)) {
						return;
					}

					auto& resolvedShader = resourceManagers.mShaderManager.resolveDefines(bloomUpShaderHandle, {});
					resolvedShader.bind();
					resolvedShader.bindUniform("filterRadius", parameters.mRadius);
					resolvedShader.bindTexture("inputTexture", resourceManagers.mTextureManager.resolve(bloomTextures[i]));

					const auto& quadMesh = resourceManagers.mMeshManager.resolve("quad");
					quadMesh.draw();

					glDisable(GL_BLEND);
					glEnable(GL_DEPTH_TEST);
					glPolygonMode(GL_FRONT_AND_BACK, oldPolygonMode);
					});
			}
		}

		// Create a new full-res render target
		auto bloomOutputHandle = resourceManagers.mFramebufferManager.asyncLoad(
			HashedString("BloomOutput"),
			FramebufferBuilder{}
			.setSize(dimension)
			.attach({ types::texture::Target::Texture2D, types::texture::InternalFormats::RGB16_F }),
			resourceManagers.mTextureManager
		);
		// Mix results
		renderPasses.renderPass(bloomOutputHandle, dimension, [inputTextureHandle, bloomTextures](const ResourceManagers& resourceManagers, const ECS& ecs) {
			auto bloomMixShaderHandle = resourceManagers.mShaderManager.asyncLoad("BloomMix Shader", SourceShader::ConstructionArgs{
				{ types::shader::Stage::Vertex, "quad.vert"},
				{ types::shader::Stage::Fragment, "bloomMix.frag" }
				});
			if (!resourceManagers.mShaderManager.isValid(bloomMixShaderHandle)) {
				return;
			}

			glDisable(GL_DEPTH_TEST);
			int oldPolygonMode;
			glGetIntegerv(GL_POLYGON_MODE, &oldPolygonMode);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

			auto bloomMixShader = resourceManagers.mShaderManager.resolveDefines(bloomMixShaderHandle, {});
			bloomMixShader.bind();
			bloomMixShader.bindTexture("bloomResults", resourceManagers.mTextureManager.resolve(bloomTextures[0]));
			bloomMixShader.bindTexture("hdrColor", resourceManagers.mTextureManager.resolve(inputTextureHandle));

			const auto& quadMesh = resourceManagers.mMeshManager.resolve("quad");
			quadMesh.draw();

			glEnable(GL_DEPTH_TEST);
			glPolygonMode(GL_FRONT_AND_BACK, oldPolygonMode);
			});

		return bloomOutputHandle;
	}
}