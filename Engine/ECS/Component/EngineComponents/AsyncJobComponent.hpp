#pragma once

#include "ECS/Component/Component.hpp"

namespace neo {

	START_COMPONENT(AsyncJobComponent);
		AsyncJobComponent(uint32_t pid) :
			mPid(pid)
		{}

		uint32_t mPid = 0;

	END_COMPONENT();
}
