#pragma once

#include "ECS/Component/Component.hpp"

namespace neo {

	struct MouseRayComponent : public Component {
		glm::vec3 mPosition;
		glm::vec3 mDirection;

		virtual std::string getName() const override {
			return "MouseRayComponent";
		}
	};
}
