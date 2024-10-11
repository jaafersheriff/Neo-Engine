#pragma once

#include "ECS/Component/Component.hpp"

#include "ResourceManager/TextureManager.hpp"

namespace neo {

	START_COMPONENT(PointLightShadowMapComponent);
	PointLightShadowMapComponent(int resolution, const TextureManager& textureManager)
		: mID("ShadowMap_" + std::to_string(util::genRandom()))
	{
		mShadowMap = textureManager.asyncLoad(
			HashedString(mID.c_str()),
			TextureBuilder{}
			.setDimension(glm::u16vec3(static_cast<uint16_t>(resolution), static_cast<uint16_t>(resolution), 0))
			.setFormat(
				TextureFormat { 
					types::texture::Target::TextureCube, 
					types::texture::InternalFormats::D16,
					TextureFilter { types::texture::Filters::Linear, types::texture::Filters::Linear },
					TextureWrap{ types::texture::Wraps::Repeat, types::texture::Wraps::Repeat, types::texture::Wraps::Repeat }
				}
			)
		);
	}

		TextureHandle mShadowMap;
		std::string mID;
	END_COMPONENT();
}