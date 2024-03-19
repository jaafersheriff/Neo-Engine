#pragma once

#include "ECS/Component/Component.hpp"

namespace neo {
	struct PhongShaderComponent : public Component {
		virtual std::string getName() const override {
			return "PhongShaderComponent";
		}
	};
}