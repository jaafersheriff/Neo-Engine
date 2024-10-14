#pragma once

#include <string>

#include "Util/Util.hpp"

namespace neo {

	class ECS;
	class ResourceManagers;

	class System {

		public:
			System(const std::string & name) :
				mName(name)
			{}
			~System() = default;
			System(const System&) = delete;
			System& operator=(const System&) = delete;

			virtual void init(ECS& ecs) {
				NEO_UNUSED(ecs);
			}

			virtual void update(ECS& ecs, const ResourceManagers& resourceManagers) {
				NEO_UNUSED(ecs, resourceManagers);
			}

			virtual void imguiEditor(ECS& ecs) {
				NEO_UNUSED(ecs);
			}

			bool mActive = true;
			const std::string mName = 0;
	};
}