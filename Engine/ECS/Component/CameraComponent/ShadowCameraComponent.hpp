#pragma once

#include "ECS/Component/Component.hpp"

namespace neo
{
	START_COMPONENT(ShadowCameraComponent);
	ShadowCameraComponent(ECS::Entity entity, types::texture::Target target, int resolution, const TextureManager& textureManager) 
		: mID("ShadowMap_" + std::to_string(static_cast<uint32_t>(entity)))
	{
		NEO_ASSERT(target == types::texture::Target::Texture2D || target == types::texture::Target::TextureCube, "Shadows can only be 2D or cube");
		mShadowMap = textureManager.asyncLoad(
			HashedString(mID.c_str()),
			TextureBuilder{}
			.setDimension(glm::u16vec3(static_cast<uint16_t>(resolution), static_cast<uint16_t>(resolution), 0))
			.setFormat(TextureFormat{ target, types::texture::InternalFormats::D16 })
		);
	}

		TextureHandle mShadowMap;
		std::string mID;
	END_COMPONENT();
}