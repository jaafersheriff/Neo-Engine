#pragma once

#include "ECS/Component/Component.hpp"

using namespace neo;

namespace PBR {
	struct IBLComponent : public Component {
		IBLComponent() {}

		TextureHandle mConvolvedSkybox = NEO_INVALID_HANDLE;
		uint16_t mConvolvedCubemapResolution = 512;

		TextureHandle mDFGLut = NEO_INVALID_HANDLE;
		uint16_t mDFGLutResolution = 64;

		virtual std::string getName() const override {
			return "IBLComponent";
		}
	};
}