#pragma once

#include "Renderer/GLObjects/SourceShader.hpp"
#include "Renderer/GLObjects/ResolvedShaderInstance.hpp"
#include "Renderer/FrameGraph/FrameGraph.hpp"

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

	inline FramebufferHandle bloom(
		FrameGraph& fg,
		FramebufferHandle outTarget,
		Viewport mainViewport,
		const ResourceManagers& resourceManagers, 
		const BloomParameters& parameters) {
		TRACY_ZONE();

		NEO_ASSERT(parameters.mDownSampleSteps > 0, "Gotta bloom with something");

		TextureHandle inputTexture;
		{
			if (!resourceManagers.mFramebufferManager.isValid(outTarget)) {
				return outTarget;
			}
			const auto& target = resourceManagers.mFramebufferManager.resolve(outTarget);
			if (!resourceManagers.mTextureManager.isValid(target.mTextures[0])) {
				return outTarget;
			}
			inputTexture = target.mTextures[0];
		}

		// Create textures and targets
		std::vector<FramebufferHandle> bloomTargets;
		std::string targetName = "BloomTargetN";
		glm::uvec2 baseDimension = glm::uvec2(mainViewport.z, mainViewport.w) / glm::uvec2(2);
		for (int i = 0; i < parameters.mDownSampleSteps; i++) {
			targetName.back() = static_cast<char>(static_cast<int>('0') + i);
			bloomTargets.push_back(resourceManagers.mFramebufferManager.asyncLoad(
				HashedString(targetName.c_str()),
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

			PassState passState;
			passState.mDepthTest = false;
			for (int i = 0; i < parameters.mDownSampleSteps; i++) {
				fg.clear(bloomTargets[i], glm::vec4(0.f, 0.f, 0.f, 1.f), types::framebuffer::AttachmentBit::Color);

				glm::uvec2 mipDimension(baseDimension.x >> i, baseDimension.y >> i);
				Viewport vp(0, 0, mipDimension);
				fg.pass(bloomTargets[i], vp, vp, passState, bloomDownShaderHandle)
					.with([parameters, i, inputTexture, mipDimension, bloomTargets](Pass& pass, const ResourceManagers& resourceManagers, const ECS&) {
					MakeDefine(MIP_0);
					if (i == 0) {
						pass.setDefine(MIP_0);
						pass.bindTexture("inputTexture", inputTexture);
						pass.bindUniform("threshold", parameters.mLuminanceThreshold);
					}
					else if (resourceManagers.mFramebufferManager.isValid(bloomTargets[i - 1])) {
						pass.bindTexture("inputTexture", resourceManagers.mFramebufferManager.resolve(bloomTargets[i - 1]).mTextures[0]);
					}
					else {
						return;
					}
					pass.bindUniform("texelSize", glm::vec2(1.f / glm::vec2(mipDimension)));
					pass.drawCommand(MeshHandle("quad"), {}, {});
						})
					.dependsOn(i == 0 ? outTarget : bloomTargets[i-1])
					.setDebugName("Bloom Down");
			}
		}

		// Up sample
		{
			auto bloomUpShaderHandle = resourceManagers.mShaderManager.asyncLoad("BloomUp Shader", SourceShader::ConstructionArgs{
				{ types::shader::Stage::Vertex, "quad.vert"},
				{ types::shader::Stage::Fragment, "bloomUp.frag" }
				});

			PassState passState;
			passState.mDepthTest = false;
			passState.mBlending = true;
			passState.mBlendSrcRGB = passState.mBlendSrcAlpha = types::passState::BlendFactor::One;
			passState.mBlendDstRGB = passState.mBlendDstAlpha = types::passState::BlendFactor::One;
			passState.mBlendEquation = types::passState::BlendEquation::Add;
			for (int i = parameters.mDownSampleSteps - 1; i > 0; i--) {
				glm::uvec2 mipDimension(baseDimension.x >> (i - 1), baseDimension.y >> (i - 1));
				Viewport vp(0, 0, mipDimension.x, mipDimension.y);
				fg.pass(bloomTargets[i - 1], vp, vp, passState, bloomUpShaderHandle)
					.with([parameters, i, bloomTargets](Pass& pass, const ResourceManagers& resourceManagers, const ECS&) {
					if (!resourceManagers.mFramebufferManager.isValid(bloomTargets[i])) {
						return;
					}
					const auto& bloomTarget = resourceManagers.mFramebufferManager.resolve(bloomTargets[i]);
					if (bloomTarget.mTextures.empty()) {
						return;
					}
					pass.bindUniform("filterRadius", parameters.mRadius);
					pass.bindTexture("inputTexture", bloomTarget.mTextures[0]);
					pass.drawCommand(MeshHandle("quad"), {}, {});
						})
					.dependsOn(bloomTargets[i])
					.setDebugName("Bloom Up")
					;
			}
		}

		{
			// Create a new full-res render target
			auto bloomOutputHandle = resourceManagers.mFramebufferManager.asyncLoad(
				HashedString("BloomOutput"),
				FramebufferBuilder{}
				.setSize(glm::uvec2(mainViewport.z, mainViewport.w))
				.attach({ types::texture::Target::Texture2D, types::texture::InternalFormats::RGB16_F }),
				resourceManagers.mTextureManager
			);

			auto bloomMixShaderHandle = resourceManagers.mShaderManager.asyncLoad("BloomMix Shader", SourceShader::ConstructionArgs{
				{ types::shader::Stage::Vertex, "quad.vert"},
				{ types::shader::Stage::Fragment, "bloomMix.frag" }
				});

			PassState passState;
			passState.mDepthTest = false;
			fg.pass(bloomOutputHandle, mainViewport, mainViewport, passState, bloomMixShaderHandle)
				.with([bloomTargets, inputTexture](Pass& pass, const ResourceManagers& resourceManagers, const ECS&) {
					if (!resourceManagers.mFramebufferManager.isValid(bloomTargets[0])) {
						return;
					}
					const Framebuffer& bloomResults = resourceManagers.mFramebufferManager.resolve(bloomTargets[0]);
					if (bloomResults.mTextures.empty()) {
						return;
					}
					pass.bindTexture("bloomResults", bloomResults.mTextures[0]);
					pass.bindTexture("hdrColor", inputTexture);
					pass.drawCommand(MeshHandle("quad"), {}, {});
				})
				.dependsOn(inputTexture, bloomTargets[0])
				.setDebugName("Bloom final")
			;

			return bloomOutputHandle;
		}
	}
}