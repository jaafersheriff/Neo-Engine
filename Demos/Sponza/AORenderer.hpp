#pragma once

#include "Renderer/GLObjects/GLHelper.hpp"
#include "Renderer/GLObjects/Framebuffer.hpp"
#include "Renderer/GLObjects/ResolvedShaderInstance.hpp"

#include "ECS/ECS.hpp"
#include "ECS/Component/CameraComponent/MainCameraComponent.hpp"
#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"

#include "Loader/Library.hpp"
#include "ResourceManager/MeshResourceManager.hpp"

#include "Util/Util.hpp"

using namespace neo;

namespace Sponza {

	namespace {
		void _generateKernel(const neo::TextureResourceManager& textureManager, HashedString id, uint32_t size) {
			std::vector<float> kernel;
			for (unsigned i = 0; i < size; i++) {
				glm::vec3 sample(
					util::genRandom(-1.f, 1.f),
					util::genRandom(-1.f, 1.f),
					util::genRandom(0.f, 1.f)
				);
				sample = glm::normalize(sample);
				sample *= util::genRandom(0.f, 1.f);
				float scale = (float)i / (float)size;
				scale = util::lerp(0.1f, 1.f, scale * scale);
				sample *= scale;
				// texture is 32bit, data upload is 8bit
				kernel.push_back(sample.x);
				kernel.push_back(sample.y);
				kernel.push_back(sample.z);
			};
			TextureBuilder builder;
			builder.mFormat.mTarget = types::texture::Target::Texture1D;
			builder.mFormat.mInternalFormat = types::texture::InternalFormats::RGB32_F;
			builder.mFormat.mFilter = {
				types::texture::Filters::Nearest,
				types::texture::Filters::Nearest,
			};
			builder.mFormat.mWrap = {
				types::texture::Wraps::Repeat,
				types::texture::Wraps::Repeat,
			};
			builder.mFormat.mType = types::ByteFormats::UnsignedByte;
			builder.mDimensions = glm::uvec3(size, 0, 0);
			builder.mData = reinterpret_cast<uint8_t*>(kernel.data());
			NEO_UNUSED(textureManager.asyncLoad(id, builder));
		}

		void _generateNoise(const neo::TextureResourceManager& textureManager, HashedString id, uint32_t dim) {
			std::vector<float> noise;
			noise.resize(dim * dim * 3);
			for (unsigned i = 0; i < dim * dim * 3; i += 3) {
				noise[i + 0] = util::genRandom();
				noise[i + 1] = util::genRandom();
				noise[i + 2] = util::genRandom();
			}
			TextureBuilder builder;
			builder.mFormat = TextureFormat{
				types::texture::Target::Texture2D,
				types::texture::InternalFormats::RGB32_F,
				{
					types::texture::Filters::Nearest,
					types::texture::Filters::Nearest,
				},
				{
					types::texture::Wraps::Repeat,
					types::texture::Wraps::Repeat,
				},
				types::ByteFormats::UnsignedByte
			};
			builder.mDimensions = glm::uvec3(dim, dim, 0);
			builder.mData = reinterpret_cast<uint8_t*>(noise.data());
			NEO_UNUSED(textureManager.asyncLoad(id, builder));
		}
	}

	TextureHandle drawAO(const ResourceManagers& resourceManagers, const ECS& ecs, ECS::Entity cameraEntity, const Framebuffer& gbuffer, glm::uvec2 targetSize, float radius, float bias) {
		TRACY_GPU();
		HashedString aoKernelHandle("aoKernel");
		HashedString aoNoiseHandle("aoNoise");
		if (!resourceManagers.mTextureManager.isValid(aoKernelHandle)) {
			_generateKernel(resourceManagers.mTextureManager, aoKernelHandle, 8);
		}
		if (!resourceManagers.mTextureManager.isValid(aoNoiseHandle)) {
			_generateNoise(resourceManagers.mTextureManager, aoNoiseHandle, 4);
		}

		// Do base AO at half res
		auto baseAOTexture = resourceManagers.mTextureManager.asyncLoad("AO Base",
			TextureBuilder{
				TextureFormat {
					types::texture::Target::Texture2D,
					types::texture::InternalFormats::R16_F,
					{
						types::texture::Filters::Linear,
						types::texture::Filters::Linear,
					},
					{
						types::texture::Wraps::Repeat,
						types::texture::Wraps::Repeat,
					},
					types::ByteFormats::Float
				},
				glm::u16vec3(glm::max(glm::uvec2(1, 1), targetSize / 2u), 0.0)
			}
		);
		auto blurAOTexture = resourceManagers.mTextureManager.asyncLoad("AO Blur",
			TextureBuilder{
				TextureFormat {
				types::texture::Target::Texture2D,
				types::texture::InternalFormats::R16_F,
				{
					types::texture::Filters::Linear,
					types::texture::Filters::Linear,
				},
				{
					types::texture::Wraps::Repeat,
					types::texture::Wraps::Repeat,
				},
				types::ByteFormats::Float
				},
				glm::u16vec3(targetSize, 0.0)
			}
		);

		// Make a one-off framebuffer for the base AO
		{
			TRACY_GPUN("Base AO");
			auto baseAOHandle = resourceManagers.mFramebufferManager.asyncLoad(resourceManagers.mTextureManager,
				"AO Base",
				std::vector<TextureHandle>{baseAOTexture}
			);
			if (resourceManagers.mFramebufferManager.isValid(baseAOHandle)) {
				auto& baseAOTarget = resourceManagers.mFramebufferManager.resolve(baseAOHandle);
				baseAOTarget.clear(glm::vec4(0.f), types::framebuffer::AttachmentBit::Color);
				glViewport(0, 0, targetSize.x / 2u, targetSize.y / 2u);
				auto aoShader = resourceManagers.mShaderManager.asyncLoad("AOShader", SourceShader::ConstructionArgs{
					{ ShaderStage::VERTEX, "quad.vert"},
					{ ShaderStage::FRAGMENT, "sponza/ao.frag" }
					});
				auto& resolvedShader = resourceManagers.mShaderManager.resolveDefines(aoShader, {});
				resolvedShader.bind();

				resolvedShader.bindUniform("radius", radius);
				resolvedShader.bindUniform("bias", bias);

				// bind gbuffer
				resolvedShader.bindTexture("gNormal", resourceManagers.mTextureManager.resolve(gbuffer.mTextures[2]));
				resolvedShader.bindTexture("gDepth", resourceManagers.mTextureManager.resolve(gbuffer.mTextures[3]));

				// bind kernel and noise
				resolvedShader.bindTexture("noise", resourceManagers.mTextureManager.resolve(aoKernelHandle));
				resolvedShader.bindTexture("kernel", resourceManagers.mTextureManager.resolve(aoNoiseHandle));

				const auto P = ecs.cGetComponentAs<CameraComponent, PerspectiveCameraComponent>(cameraEntity)->getProj();
				resolvedShader.bindUniform("P", P);
				resolvedShader.bindUniform("invP", glm::inverse(P));

				resourceManagers.mMeshManager.resolve("quad").draw();
			}
		}
		{
			TRACY_GPUN("AO Blur");
			{
				auto blurAOHandle = resourceManagers.mFramebufferManager.asyncLoad(resourceManagers.mTextureManager,
					"AO Base",
					std::vector<TextureHandle>{blurAOTexture}
				);
				if (resourceManagers.mTextureManager.isValid(baseAOTexture) && resourceManagers.mFramebufferManager.isValid(blurAOHandle)) {
					auto blurredAO = resourceManagers.mFramebufferManager.resolve(blurAOHandle);
					blurredAO.bind();
					blurredAO.clear(glm::vec4(0.f), types::framebuffer::AttachmentBit::Color);
					glViewport(0, 0, targetSize.x, targetSize.y);

					auto blurShader = resourceManagers.mShaderManager.asyncLoad("BlurShader", SourceShader::ConstructionArgs{
						{ ShaderStage::VERTEX, "quad.vert"},
						{ ShaderStage::FRAGMENT, "sponza/blur.frag" }
						});

					auto& resolvedShader = resourceManagers.mShaderManager.resolveDefines(blurShader, {});
					resolvedShader.bind();

					resolvedShader.bindTexture("inputAO", resourceManagers.mTextureManager.resolve(baseAOTexture));
					resolvedShader.bindUniform("blurAmount", 2);

					resourceManagers.mMeshManager.resolve("quad").draw();
				}
			}
		}
		return blurAOTexture;
	}

}