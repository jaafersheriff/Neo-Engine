#pragma once

#include "ECS/Component/Component.hpp"

namespace neo {

	struct MainLightComponent : public Component {
		virtual std::string getName() const override { return "MainLightComponent"; }
	};
}
