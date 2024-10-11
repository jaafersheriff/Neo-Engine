#pragma once

#include "ECS/Component/Component.hpp"

#include "ResourceManager/TextureManager.hpp"

namespace neo {
	START_COMPONENT(CSMShadowMapComponent);
	CSMShadowMapComponent(int resolution, const TextureManager& textureManager)
		: mID("CSMShadowMap_" + std::to_string(util::genRandom()))
	{
		mShadowMap = textureManager.asyncLoad(
			HashedString(mID.c_str()),
			TextureBuilder{}
			.setDimension(glm::u16vec3(static_cast<uint16_t>(resolution), static_cast<uint16_t>(resolution), 0))
			.setFormat(
				TextureFormat { 
					types::texture::Target::Texture2D, 
					types::texture::InternalFormats::D16,
					TextureFilter { types::texture::Filters::LinearMipmapNearest, types::texture::Filters::Linear },
					TextureWrap{ types::texture::Wraps::Clamp, types::texture::Wraps::Clamp, types::texture::Wraps::Clamp },
					types::ByteFormats::UnsignedByte,
					4 // 4 mips!
				}
			)
		);
	}

		TextureHandle mShadowMap;
		std::string mID;
	END_COMPONENT();
}