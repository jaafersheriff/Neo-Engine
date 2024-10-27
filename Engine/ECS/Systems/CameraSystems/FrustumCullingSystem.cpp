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
		// This is risky
		uint32_t cameraCount = ecs.entityCount<FrustumComponent>();

		CameraCulledComponent::CameraIDs cameraIDs;
		cameraIDs.resize(cameraCount);

		for (auto&& [entity, spatial, bb] : ecs.getView<SpatialComponent, BoundingBoxComponent>().each()) {
			// Reset the camera ID
			for (auto camera = cameraIDs.begin(); camera < cameraIDs.end(); camera++) {
				*camera = static_cast<ECS::Entity>(std::numeric_limits<uint32_t>::max());
			}

			// Populate the cameraIDs
			int i = 0;
			for (auto&& [cameraEntity, frustum, _] : cameras.each()) {
				if (frustum.isInFrustum(spatial, bb)) {
					if (cameraIDs.size() <= i) {
						cameraIDs.emplace_back(cameraEntity);
					}
					else {
						cameraIDs[i++] = cameraEntity;
					}
				}
				else {
					mCulledCount++;
				}
			}

			// Shove it into ECS
			if (auto* existingComp = ecs.getComponent<CameraCulledComponent>(entity)) {
				existingComp->mCameraIDs = cameraIDs;
			}
			else {
				ecs.addComponent<CameraCulledComponent>(entity, cameraIDs);
			}
		}
	}

	void FrustumCullingSystem::imguiEditor(ECS&) {
		ImGui::Text("Culled draws: %d", mCulledCount);
	}
}