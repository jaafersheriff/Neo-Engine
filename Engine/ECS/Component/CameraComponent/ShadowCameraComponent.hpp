#pragma once

#include "ECS/Component/Component.hpp"

namespace neo
{
	START_COMPONENT(ShadowCameraComponent);
		// Has to be mutable so that Demo::render() can write to it ;(
		mutable TextureHandle mShadowMap;
		uint16_t mResolution = 2048;
	END_COMPONENT();
}