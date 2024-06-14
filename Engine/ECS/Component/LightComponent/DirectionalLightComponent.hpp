
#pragma once

#include "ECS/Component/Component.hpp"

namespace neo {

	struct DirectionalLightComponent : public Component {
		DirectionalLightComponent() {}
		
		virtual std::string getName() const override {
			return "DirectionalLightComponent";
		}
	};
}
