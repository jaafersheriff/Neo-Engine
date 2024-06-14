#pragma once

#include "ECS/Component/Component.hpp"

namespace neo {
	struct OpaqueComponent : public Component {

		virtual std::string getName() const override {
			return "OpaqueComponent";
		}

	};
}
