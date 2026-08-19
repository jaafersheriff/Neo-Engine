#include "ECS/pch.hpp"
#include "FrustumCullingSystem.hpp"

#include "ECS/ECS.hpp"
#include "ECS/Component/CameraComponent/MainCameraComponent.hpp"
#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/CameraComponent/FrustumComponent.hpp"
#include "ECS/Component/CollisionComponent/CameraCulledComponent.hpp"

#include "ECS/Systems/CameraSystems/FrustumSystem.hpp"


namespace neo {

	void FrustumCullingSystem::update(ECS& ecs, const ResourceManagers& resourceManagers) {
		NEO_UNUSED(resourceManagers);

		TRACY_ZONEN("FrustumCullingSystem");
		NEO_ASSERT(ecs.isSystemEnabled<FrustumSystem>(), "This system can only be used with the FrustumSystem!");
		mCulledCount = 0;

		const auto& cameras = ecs.getView<FrustumComponent, CameraComponent>();
		const uint32_t cameraCount = ecs.entityCount<FrustumComponent>();
		NEO_ASSERT(cameraCount <= CameraCulledComponent::kMaxCameras,
			"Scene has %d frustums but CameraCulledComponent only tracks %d - the excess will never cull",
			cameraCount, CameraCulledComponent::kMaxCameras);

		// Reused across entities: only the first `count` entries are ever read, so there is nothing
		// to reset between iterations.
		CameraCulledComponent::CameraIDs cameraIDs = {};

		for (auto&& [entity, spatial, bb] : ecs.getView<SpatialComponent, BoundingBoxComponent>().each()) {
			uint8_t count = 0;
			for (auto&& [cameraEntity, frustum, _] : cameras.each()) {
				if (frustum.isInFrustum(spatial, bb)) {
					if (count < CameraCulledComponent::kMaxCameras) {
						cameraIDs[count++] = cameraEntity;
					}
				}
				else {
					mCulledCount++;
				}
			}

			// Shove it into ECS
			if (auto* existingComp = ecs.getComponent<CameraCulledComponent>(entity)) {
				existingComp->mCameraIDs = cameraIDs;
				existingComp->mCameraCount = count;
			}
			else {
				ecs.addComponent<CameraCulledComponent>(entity, cameraIDs, count);
			}
		}
	}

	void FrustumCullingSystem::imguiEditor(ECS&) {
		ImGui::Text("Culled draws: %d", mCulledCount);
	}
}