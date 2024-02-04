#pragma once

#include "ECS/Component/Component.hpp"

using namespace neo;

namespace FrustaFitting {
	struct MockCameraComponent : public Component {
		MockCameraComponent()
		{}

		virtual std::string getName() const override {
			return "MockCameraComponent";
		}

	};
}