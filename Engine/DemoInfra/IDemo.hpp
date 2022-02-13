#pragma once

#include "Util/Util.hpp"
#include <string>

namespace neo {

	class ECS;

	class IDemo {
		struct Config {
			std::string name = "";
		};

	public:
		IDemo() = default;
		~IDemo() = default;
		IDemo(const IDemo&) = delete;
		IDemo& operator=(const IDemo&) = delete;

		virtual void init(ECS& ecs) = 0;
		virtual void update(ECS& ecs) {
			NEO_UNUSED(ecs);
		};
		virtual void destroy() {}
		const Config getConfig() const { return mConfig; }

	protected:
		Config mConfig;
	private:
	};

}
