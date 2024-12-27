#include "ShadowMapComponents.hpp"
#include "ECS/Component/CameraComponent/CSMCameraComponent.hpp"

namespace neo {
	namespace {
		TextureHandle _createShadowMap(const TextureManager& textureManager, const char* handle, types::texture::Target target, int resolution, uint8_t mips) {
			return textureManager.asyncLoad(
				HashedString(handle),
				TextureBuilder{}
				.setDimension(glm::u16vec3(static_cast<uint16_t>(resolution), static_cast<uint16_t>(resolution), 0))
				.setFormat(
					TextureFormat{
						target,
						types::texture::InternalFormats::D16,
						TextureFilter { types::texture::Filters::Nearest, types::texture::Filters::Nearest, mips > 1 ? types::texture::Filters::Linear : types::texture::Filters::Nearest},
						TextureWrap{ types::texture::Wraps::Clamp, types::texture::Wraps::Clamp, types::texture::Wraps::Clamp },
						types::ByteFormats::UnsignedByte,
						mips
					}
				)
			);
		}
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
			CSM_CAMERA_COUNT
		);
	}
}