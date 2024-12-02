#pragma once

#include "ECS/Component/Component.hpp"

#include "ResourceManager/TextureManager.hpp"

namespace neo {
	START_COMPONENT(PointLightShadowMapComponent);
	PointLightShadowMapComponent(int resolution, const TextureManager& textureManager);
	TextureHandle mShadowMap;
	END_COMPONENT();

	START_COMPONENT(CSMShadowMapComponent);
	CSMShadowMapComponent(int resolution, const TextureManager& textureManager);
	TextureHandle mShadowMap;
	END_COMPONENT();

}