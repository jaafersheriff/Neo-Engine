#pragma once

#include "ECS/Component/Component.hpp"

namespace neo {

	START_COMPONENT(AsyncJobComponent);
		AsyncJobComponent(std::thread::id pid) :
			mPid(pid)
		{}

		const std::thread::id mPid;

	END_COMPONENT();
}
