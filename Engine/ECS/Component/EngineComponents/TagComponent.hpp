#pragma once

#include "ECS/Component/Component.hpp"

namespace neo {

	struct TagComponent : public Component {
		TagComponent() = default;
		TagComponent(std::string tag) :
			mTag(tag)
		{}

		std::string mTag;

		virtual std::string getName() const override {
			return "Tag Component";
		}
	};
}
