#include "ECS/pch.hpp"
#include "Engine/Engine.hpp"
#include "FrustumSystem.hpp"

#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/CameraComponent/FrustumComponent.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"

namespace neo {
	void FrustumSystem::update(ECS& ecs, const ResourceManagers& resourceManagers) {
		NEO_UNUSED(resourceManagers);

		TRACY_ZONEN("FrustumSystem");
		for (auto&& [entity, frustum, spatial, camera] : ecs.getView<FrustumComponent, SpatialComponent, CameraComponent>().each()) {
			frustum.calculateFrustum(camera, spatial);
		}
	}
}
