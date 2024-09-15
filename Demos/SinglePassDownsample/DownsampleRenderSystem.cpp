#include "DownsampleRenderSystem.hpp"

#include "ResourceManager/ResourceManagers.hpp"

#include "Renderer/RenderingSystems/Blitter.hpp"

using namespace neo;
namespace SPD {

	TextureHandle downSample(TextureHandle inputDepthHandle, const ResourceManagers& resourceManagers) {
		// Get input depth details
		if (!resourceManagers.mTextureManager.isValid(inputDepthHandle)) {
			return NEO_INVALID_HANDLE;
		}
		auto& inputDepth = resourceManagers.mTextureManager.resolve(inputDepthHandle);
		NEO_ASSERT(inputDepth.mFormat.mTarget == types::texture::Target::Texture2D, "Input depth needs to be 2D");
		NEO_ASSERT(inputDepth.mFormat.mInternalFormat >= types::texture::InternalFormats::D16 &&
			inputDepth.mFormat.mInternalFormat <= types::texture::InternalFormats::D24S8, "Hi z expects depth buffer");

		auto outputHandle = resourceManagers.mTextureManager.asyncLoad("hi z", TextureBuilder{
			TextureFormat {
				types::texture::Target::Texture2D,
				types::texture::InternalFormats::R16_F,
				TextureFilter {
					types::texture::Filters::NearestMipmapNearest,
					types::texture::Filters::Nearest
				},
				TextureWrap{ 
					types::texture::Wraps::Clamp, 
					types::texture::Wraps::Clamp, 
					types::texture::Wraps::Clamp 
				},
				types::ByteFormats::Float,
				UINT8_MAX // this will auto resolve in the back to max # of mips 
			},
			glm::u16vec3(inputDepth.mWidth, inputDepth.mHeight, 0)
		});
		if (!resourceManagers.mTextureManager.isValid(outputHandle)) {
			return NEO_INVALID_HANDLE;
		}
		auto& outputDepth = resourceManagers.mTextureManager.resolve(outputHandle);

		auto downsampleShaderHandle = resourceManagers.mShaderManager.asyncLoad("Downsample Compute", SourceShader::ConstructionArgs{
			{types::shader::Stage::Compute, "spd/spd.compute"}
		});
		if (!resourceManagers.mShaderManager.isValid(downsampleShaderHandle)) {
			return NEO_INVALID_HANDLE;
		}

		auto& downsamplerShader = resourceManagers.mShaderManager.resolveDefines(downsampleShaderHandle, {});
		downsamplerShader.bindTexture("src", inputDepth);
		for (int i = 1; i < outputDepth.mFormat.mMipCount; i++) {
			downsamplerShader.bindUniform("textureDimensions", glm::vec2(inputDepth.mWidth << (i-1), inputDepth.mHeight << (i-1)));
			downsamplerShader.bindUniform("currentMip", i);
			auto barrier = downsamplerShader.bindImageTexture("dst", outputDepth, types::shader::Access::ReadWrite, i);

			downsamplerShader.dispatch({ (inputDepth.mWidth << i) / 32, (inputDepth.mHeight << i) / 32, 1 });
		}

		return outputHandle;
	}

	void downSampleDebugBlit(Framebuffer& outputFBO, TextureHandle hiz, const ResourceManagers& resourceManagers) {
		if (!resourceManagers.mTextureManager.isValid(hiz)) {
			return;
		}
		auto& hizTexture = resourceManagers.mTextureManager.resolve(hiz);

		blit(resourceManagers, outputFBO, hiz, glm::uvec2(hizTexture.mWidth, hizTexture.mHeight), glm::vec4(0.f, 0.f, 0.f, 1.f), 0);
	}
}