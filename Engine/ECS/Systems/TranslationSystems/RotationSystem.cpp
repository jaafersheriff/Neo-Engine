#include "ECS/pch.hpp"
#include "RotationSystem.hpp"
#include "ECS/ECS.hpp"

#include "ECS/Component/EngineComponents/FrameStatsComponent.hpp"
#include "ECS/Component/SpatialComponent/RotationComponent.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"

namespace neo {

	void RotationSystem::update(ECS& ecs, const ResourceManagers& resourceManagers) {
		NEO_UNUSED(resourceManagers);

		TRACY_ZONEN("RotationSystem");

		if (auto frameStatsOpt = ecs.getComponent<FrameStatsComponent>()) {
			auto&& [_, frameStats] = *frameStatsOpt;
			for (auto&& [entity, rotation, spatial] : ecs.getView<RotationComponent, SpatialComponent>().each()) {
				glm::mat4 R(1.f);
				R *= glm::rotate(glm::mat4(1.f), frameStats.mDT * rotation.mSpeed.x, glm::vec3(1, 0, 0));
				R *= glm::rotate(glm::mat4(1.f), frameStats.mDT * rotation.mSpeed.y, glm::vec3(0, 1, 0));
				R *= glm::rotate(glm::mat4(1.f), frameStats.mDT * rotation.mSpeed.z, glm::vec3(0, 0, 1));
				spatial.rotate(glm::mat3(R));
			}
		}
	}
}