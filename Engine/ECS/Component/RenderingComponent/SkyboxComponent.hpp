#pragma once

#include "ShaderComponent.hpp"

#include "Loader/Library.hpp"

namespace neo {
	struct SkyboxComponent : public Component {
		SkyboxComponent(Texture* skybox) :
			mSkybox(skybox)
		{}

		Texture* mSkybox;

		virtual std::string getName() const override {
			return "SkyboxComponent";
		}
	};
}