#pragma once

#include "ECS/Component/Component.hpp"

#include "ResourceManager/TextureManager.hpp"

namespace CSM {
	using namespace neo;

	START_COMPONENT(CSMShadowMapComponent);
	CSMShadowMapComponent(types::texture::Target target, int resolution, const TextureManager& textureManager)
		: mID("CSMShadowMap_" + std::to_string(util::genRandom()))
	{
		NEO_ASSERT(target == types::texture::Target::Texture2D || target == types::texture::Target::TextureCube, "Shadows can only be 2D or cube");
		TextureWrap wrap = target == types::texture::Target::Texture2D 
			? TextureWrap{ types::texture::Wraps::Clamp, types::texture::Wraps::Clamp, types::texture::Wraps::Clamp }
			: TextureWrap{ types::texture::Wraps::Repeat, types::texture::Wraps::Repeat, types::texture::Wraps::Repeat }
		;
		mShadowMap = textureManager.asyncLoad(
			HashedString(mID.c_str()),
			TextureBuilder{}
			.setDimension(glm::u16vec3(static_cast<uint16_t>(resolution), static_cast<uint16_t>(resolution), 0))
			.setFormat(
				TextureFormat { 
					target, 
					types::texture::InternalFormats::D16,
					TextureFilter { types::texture::Filters::LinearMipmapNearest, types::texture::Filters::Linear },
					wrap,
					types::ByteFormats::UnsignedByte,
					4
				}
			)
		);
	}

		TextureHandle mShadowMap;
		std::string mID;
	END_COMPONENT();
}