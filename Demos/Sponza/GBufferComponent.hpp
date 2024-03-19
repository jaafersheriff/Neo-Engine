#pragma once

#include "ECS/Component/Component.hpp"

using namespace neo;

namespace Sponza {

	struct GBufferShaderComponent : public Component {

		virtual std::string getName() const override {
			return "GBufferShaderComponent";
		}
	};
}
