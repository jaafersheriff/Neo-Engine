#pragma once

#include "ECS/Component/Component.hpp"

#include "ResourceManager/TextureResourceManager.hpp"

namespace neo {
	struct SkyboxComponent : public Component {
		SkyboxComponent(TextureHandle skybox) :
			mSkybox(skybox)
		{}

		TextureHandle mSkybox;

		virtual std::string getName() const override {
			return "SkyboxComponent";
		}
	};
}