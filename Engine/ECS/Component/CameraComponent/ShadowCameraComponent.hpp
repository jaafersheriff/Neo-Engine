#pragma once

#include "ECS/Component/Component.hpp"

namespace neo
{
	START_COMPONENT(ShadowCameraComponent);
	ShadowCameraComponent(const char* id, types::texture::Target target, int resolution, const TextureManager& textureManager) {
		NEO_ASSERT(target == types::texture::Target::Texture2D || target == types::texture::Target::TextureCube, "Shadows can only be 2D or cube");
		mShadowMap = textureManager.asyncLoad(
			HashedString(id),
			TextureBuilder{}
			.setDimension(glm::u16vec3(static_cast<uint16_t>(resolution), static_cast<uint16_t>(resolution), 0))
			.setFormat(TextureFormat{ target, types::texture::InternalFormats::D16 })
		);
	}

		TextureHandle mShadowMap;
	END_COMPONENT();
}