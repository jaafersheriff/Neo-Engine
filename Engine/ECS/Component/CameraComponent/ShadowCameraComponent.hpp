#pragma once

#include "ECS/Component/Component.hpp"

namespace neo
{
	START_COMPONENT(ShadowCameraComponent);
		ShadowCameraComponent(int resolution = 2048)
			: mShadowMap(NEO_INVALID_HANDLE)
			, mResolution(static_cast<uint16_t>(resolution))
		{}
		// Has to be mutable so that Demo::render() can write to it ;(
		mutable TextureHandle mShadowMap;
		uint16_t mResolution = 2048;
	END_COMPONENT();
}