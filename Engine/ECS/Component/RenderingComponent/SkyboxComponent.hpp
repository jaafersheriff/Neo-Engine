#pragma once

#include "ECS/Component/Component.hpp"

#include "ResourceManager/TextureManager.hpp"

namespace neo {
	START_COMPONENT(SkyboxComponent);
		SkyboxComponent(TextureHandle skybox) 
			: mSkybox(skybox)
		{}

		TextureHandle mSkybox;
	END_COMPONENT();
}