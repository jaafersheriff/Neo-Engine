#pragma once

#include "ECS/Component/Component.hpp"

namespace neo {

	struct DebugBoundingBoxComponent : public Component {
		virtual std::string getName() const override {
			return "DebugBoundingBoxComponent";
		}
	};
}
