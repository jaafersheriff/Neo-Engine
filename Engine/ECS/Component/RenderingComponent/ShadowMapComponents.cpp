#include "ShadowMapComponents.hpp"

namespace neo {
	namespace {
		TextureHandle _createShadowMap(const TextureManager& textureManager, const char* handle, types::texture::Target target, int resolution, uint8_t mips) {
			TextureWrap wrap = target == types::texture::Target::TextureCube 
				? TextureWrap{ types::texture::Wraps::Repeat, types::texture::Wraps::Repeat, types::texture::Wraps::Repeat }
				: TextureWrap{ types::texture::Wraps::Clamp, types::texture::Wraps::Clamp, types::texture::Wraps::Clamp } 
			;
			types::texture::Filters minFilter = mips > 1
				? types::texture::Filters::NearestMipmapNearest
				: types::texture::Filters::Nearest
			;

			return textureManager.asyncLoad(
				HashedString(handle),
				TextureBuilder{}
				.setDimension(glm::u16vec3(static_cast<uint16_t>(resolution), static_cast<uint16_t>(resolution), 0))
				.setFormat(
					TextureFormat{
						target,
						types::texture::InternalFormats::D16,
						TextureFilter { minFilter, types::texture::Filters::Nearest },
						wrap,
						types::ByteFormats::UnsignedByte,
						mips
					}
				)
			);
		}
	}

	ShadowCameraComponent::ShadowCameraComponent(int resolution, const TextureManager& textureManager) {
		mShadowMap = _createShadowMap(
			textureManager,
			"ShadowMap",
			types::texture::Target::Texture2D,
			resolution,
			1
		);
	}

	PointLightShadowMapComponent::PointLightShadowMapComponent(int resolution, const TextureManager& textureManager) {
		mShadowMap = _createShadowMap(
			textureManager,
			std::string("PointLightShadowMap_" + std::to_string(util::genRandom())).c_str(),
			types::texture::Target::TextureCube,
			resolution,
			1
		);
	}

	CSMShadowMapComponent::CSMShadowMapComponent(int resolution, const TextureManager& textureManager) {
		mShadowMap = _createShadowMap(
			textureManager,
			"CSMShadowMap",
			types::texture::Target::Texture2D,
			resolution,
			4
		);
	}
}