#pragma once

#include "ECS/Component/Component.hpp"

using namespace neo;

namespace PBR {
	struct IBLComponent : public Component {
		IBLComponent() {}

		TextureHandle mConvolvedSkybox = NEO_INVALID_HANDLE;
		TextureHandle mDFGLUT = NEO_INVALID_HANDLE;

		virtual std::string getName() const override {
			return "IBLComponent";
		}
	};
}