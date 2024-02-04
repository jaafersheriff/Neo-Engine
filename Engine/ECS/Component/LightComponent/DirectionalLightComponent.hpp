
#pragma once

#include "ECS/Component/Component.hpp"

#include <string>

namespace neo {

	struct DirectionalLightComponent : public Component {
		DirectionalLightComponent() {}
		
		virtual std::string getName() const override {
			return "DirectionalLightComponent";
		}
	};
}