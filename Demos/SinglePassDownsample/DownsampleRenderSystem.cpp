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

		return resourceManagers.mTextureManager.asyncLoad("hi z", TextureBuilder{
			TextureFormat {
				types::texture::Target::Texture2D,
				inputDepth.mFormat.mInternalFormat,
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

	}

	void downSampleDebugBlit(Framebuffer& outputFBO, TextureHandle hiz, const ResourceManagers& resourceManagers) {
		if (!resourceManagers.mTextureManager.isValid(hiz)) {
			return;
		}
		auto& hizTexture = resourceManagers.mTextureManager.resolve(hiz);

		blit(resourceManagers, outputFBO, hiz, glm::uvec2(hizTexture.mWidth, hizTexture.mHeight));
	}
}