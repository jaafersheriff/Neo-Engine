#pragma once

#include "Renderer/Renderer.hpp"
#include "Renderer/GLObjects/SourceShader.hpp"
#include "Renderer/GLObjects/ResolvedShaderInstance.hpp"
#include "Renderer/RenderingSystems/RenderPass.hpp"

#include "ECS/ECS.hpp"
#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/RenderingComponent/IBLComponent.hpp"
#include "ECS/Component/RenderingComponent/SkyboxComponent.hpp"

#include "ResourceManager/ResourceManagers.hpp"

#include "Messaging/Message.hpp"
#include "Messaging/Messenger.hpp"

#include "Util/Util.hpp"
#include "Util/ServiceLocator.hpp"

#include <tuple>

namespace neo {

	void convolveCubemap(RenderPasses& renderPasses, const ResourceManagers& resourceManagers, const ECS& ecs) {
		TRACY_ZONE();

		auto skyboxTuple = ecs.getSingleView<SkyboxComponent, IBLComponent>();
		if (!skyboxTuple) {
			return;
		}
		const ECS::Entity iblEntity = std::get<0>(*skyboxTuple);
		const SkyboxComponent& skybox = std::get<1>(*skyboxTuple);
		const IBLComponent& ibl = std::get<2>(*skyboxTuple);

		if (!resourceManagers.mTextureManager.isValid(skybox.mSkybox)) {
			NEO_LOG_W("There's no skybox texture to convolve");
			return;
		}
		const Texture& skyboxCubemap = resourceManagers.mTextureManager.resolve(skybox.mSkybox);
		if (skyboxCubemap.mFormat.mMipCount < 2) {
			NEO_LOG_E("Skybox cubemap needs to have mips");
			return;
		}

		// The renderer owns the texture names - the component only remembers the handles it is sent.
		// asyncLoad is idempotent, so re-requesting each frame just hands the same handle back.
		const TextureHandle dfgLut = resourceManagers.mTextureManager.asyncLoad(
				HashedString("dfgLut"),
				TextureBuilder{}
				.setDimension(glm::u16vec3(ibl.mDFGLutResolution, ibl.mDFGLutResolution, 0))
				.setFormat(TextureFormat{
					types::texture::Target::Texture2D,
					types::texture::InternalFormats::RGBA16_F,
					TextureFilter { types::texture::Filters::Linear, types::texture::Filters::Linear },
					TextureWrap { types::texture::Wraps::Clamp, types::texture::Wraps::Clamp, types::texture::Wraps::Clamp },
					types::ByteFormats::Float
					})
		);

		if (!ibl.mDFGGenerated) {
			renderPasses.computePass([&ibl, dfgLut, iblEntity](const ResourceManagers& resourceManagers, const ECS&) {
				TRACY_GPUN("DFG LUT");
				if (resourceManagers.mTextureManager.isValid(dfgLut)) {
					auto dfgLutShaderHandle = resourceManagers.mShaderManager.asyncLoad("DFGLutShader", ShaderBuilder{}
						.setStage(types::shader::Stage::Compute, "dfglut.comp")
					);
					if (resourceManagers.mShaderManager.isValid(dfgLutShaderHandle)) {
						auto& dfgLutShader = resourceManagers.mShaderManager.resolveDefines(dfgLutShaderHandle, {});
						dfgLutShader.bind();

						auto barrier = dfgLutShader.bindImageTexture("dst", resourceManagers.mTextureManager.resolve(dfgLut), types::shader::Access::Write);
						dfgLutShader.dispatch({
							ibl.mDFGLutResolution / 8,
							ibl.mDFGLutResolution / 8,
							1
							});
						// Discovered during render, so it has to travel back to the live ECS.
						Messenger::sendMessage<IBLDFGLutGeneratedMessage>(iblEntity, dfgLut);
					}
				}
			}, "DFG LUT");
		}

		const TextureHandle convolvedSkybox = resourceManagers.mTextureManager.asyncLoad(
				HashedString("convolvedSkybox"),
				TextureBuilder{}
				.setDimension(glm::u16vec3(ibl.mConvolvedCubemapResolution, ibl.mConvolvedCubemapResolution, 0))
				.setFormat(TextureFormat{
					types::texture::Target::TextureCube,
					types::texture::InternalFormats::RGBA16_F,
					TextureFilter { types::texture::Filters::Linear, types::texture::Filters::Linear, types::texture::Filters::Linear },
					TextureWrap { types::texture::Wraps::Clamp , types::texture::Wraps::Clamp , types::texture::Wraps::Clamp  },
					types::ByteFormats::Float,
					skyboxCubemap.mFormat.mMipCount
				})
		);

		if (!ibl.mConvolved) {
			renderPasses.computePass([&ibl, &skyboxCubemap, convolvedSkybox, iblEntity](const ResourceManagers& resourceManagers, const ECS&) {
				TRACY_GPUN("Convolve");
				if (resourceManagers.mTextureManager.isValid(convolvedSkybox)) {
					auto convolveShaderHandle = resourceManagers.mShaderManager.asyncLoad("ConvolveShader", ShaderBuilder{}
						.setStage(types::shader::Stage::Compute, "convolve.comp")
					);
					if (resourceManagers.mShaderManager.isValid(convolveShaderHandle)) {
						const auto& convolvedCubemap = resourceManagers.mTextureManager.resolve(convolvedSkybox);

						ShaderDefines defines;
						MakeDefine(EQUIRECTANGULAR);
						MakeDefine(HDR);
						if (skyboxCubemap.mFormat.mTarget == types::texture::Target::Texture2D) {
							defines.set(EQUIRECTANGULAR);
						}
						if (skyboxCubemap.mFormat.mType != types::ByteFormats::UnsignedByte) {
							defines.set(HDR);
						}
						auto& convolveShader = resourceManagers.mShaderManager.resolveDefines(convolveShaderHandle, defines);
						convolveShader.bind();

						convolveShader.bindTexture("cubeMap", skyboxCubemap);
						convolveShader.bindUniform("resolution", convolvedCubemap.mWidth);
						for (int mip = 0; mip < convolvedCubemap.mFormat.mMipCount; mip++) {
							convolveShader.bindUniform("mipLevel", mip);
							uint16_t mipResolution = convolvedCubemap.mWidth >> uint16_t(mip);
							convolveShader.bindUniform("roughness", mip / static_cast<float>(convolvedCubemap.mFormat.mMipCount - 2));
							convolveShader.bindUniform("sampleCount", ibl.mSampleCount);
							auto barrier = convolveShader.bindImageTexture("dst", convolvedCubemap, types::shader::Access::Write, mip);
							convolveShader.dispatch({
								mipResolution / 8,
								mipResolution / 8,
								1
								});
						}

						Messenger::sendMessage<IBLConvolvedMessage>(iblEntity, convolvedSkybox);
					}
				}
			}, "Convolve");
		}
	}
}