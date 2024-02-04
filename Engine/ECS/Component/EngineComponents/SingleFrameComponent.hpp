#pragma once

#include "ECS/Component/Component.hpp"

namespace neo {

	struct SingleFrameComponent : public Component {
		SingleFrameComponent() {}
		virtual std::string getName() const override {
			return "SingleFrameComponent";
		}
	};
}