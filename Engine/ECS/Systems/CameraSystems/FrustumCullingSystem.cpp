#include "ECS/pch.hpp"
#include "FrustumCullingSystem.hpp"

#include "ECS/ECS.hpp"
#include "ECS/Component/CameraComponent/MainCameraComponent.hpp"
#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/CameraComponent/FrustumComponent.hpp"
#include "ECS/Component/CollisionComponent/CameraCulledComponent.hpp"

#include "ECS/Systems/CameraSystems/FrustumSystem.hpp"

#pragma optimize("", off)

namespace neo {

	void FrustumCullingSystem::update(ECS& ecs) {
		TRACY_ZONEN("FrustumCullingSystem");
		NEO_ASSERT(ecs.isSystemEnabled<FrustumSystem>(), "This system can only be used with the FrustumSystem!");
		mCulledCount = 0;

		const auto& cameras = ecs.getView<FrustumComponent, CameraComponent>();

		for (auto&& [entity, spatial, bb] : ecs.getView<SpatialComponent, BoundingBoxComponent>().each()) {
			CameraCulledComponent* component;
			if (auto* existingComp = ecs.getComponent<CameraCulledComponent>(entity)) {
				component = existingComp;
			}
			else {
				component = ecs.addComponent<CameraCulledComponent>(entity);
			}

			for (auto camera = component->mCameraViews.begin(); camera < component->mCameraViews.end(); camera++) {
				*camera = static_cast<ECS::Entity>(std::numeric_limits<uint32_t>::max());
			}

			int i = 0;
			for (auto&& [cameraEntity, frustum, _] : cameras.each()) {
				if (frustum.isInFrustum(spatial, bb)) {
					if (component->mCameraViews.size() <= i) {
						component->mCameraViews.push_back(cameraEntity);
					}
					else {
						component->mCameraViews[i++] = cameraEntity;
					}
				}
				else {
					mCulledCount++;
				}
			}
		}
	}

	void FrustumCullingSystem::imguiEditor(ECS&) {
		ImGui::Text("Culled draws: %d", mCulledCount);
	}
}