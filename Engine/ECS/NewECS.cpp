#include "ECS/NewECS.hpp"

namespace neo {

	ECS::Entity ECS::createEntity() {
		return mRegistry.create();
	}

	void ECS::removeEntity(Entity e) {
		mEntityKillQueue.push_back(e);
	}

	void ECS::flush() {
		for (auto&& job : mAddComponentFuncs) {
			job(mRegistry);
		}
		mAddComponentFuncs.clear();

		for (auto&& job : mRemoveComponentFuncs) {
			job(mRegistry);
		}
		mRemoveComponentFuncs.clear();

		for (Entity e : mEntityKillQueue) {
			mRegistry.destroy(e);
		}
		mEntityKillQueue.clear();
	}
}
