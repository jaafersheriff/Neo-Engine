#pragma once

#include "ECS/Component/Component.hpp"

namespace neo {

	struct PinnedComponent : public Component {
		PinnedComponent() = default;
		virtual std::string getName() const override {
			return "PinnedComponent";
		}
	};
}