#pragma once

#include "ECS/Component/Component.hpp"

namespace neo {
	struct AlphaTestComponent : public Component {

		virtual std::string getName() const override {
			return "AlphaTestComponent";
		}
	};
}