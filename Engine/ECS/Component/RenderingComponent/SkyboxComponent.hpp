#pragma once

#include "ECS/Component/Component.hpp"

#include "ResourceManager/TextureManager.hpp"

namespace neo {
	START_COMPONENT(SkyboxComponent);
		SkyboxComponent(TextureHandle skybox, bool equirectangular) 
			: mSkybox(skybox)
			, mEquirectangular(equirectangular)
		{}

		TextureHandle mSkybox;
		bool mEquirectangular = false;
	END_COMPONENT();
}