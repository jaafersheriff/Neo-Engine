#pragma once

#include "ECS/Component/Component.hpp"

namespace neo {

	START_COMPONENT(TagComponent);
		TagComponent(std::string tag) :
			mTag(tag)
		{}

		std::string mTag;

	END_COMPONENT();
}
