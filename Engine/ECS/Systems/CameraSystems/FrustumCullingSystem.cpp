#include "ECS/pch.hpp"
#include "FrustumCullingSystem.hpp"

#include "ECS/ECS.hpp"
#include "ECS/Component/CameraComponent/MainCameraComponent.hpp"
#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/CameraComponent/FrustumComponent.hpp"
#include "ECS/Component/CollisionComponent/CameraCulledComponent.hpp"

#include "ECS/Systems/CameraSystems/FrustumSystem.hpp"

#include <execution>

namespace neo {

	void FrustumCullingSystem::update(ECS& ecs) {
		TRACY_ZONEN("FrustumCullingSystem");
		NEO_ASSERT(ecs.isSystemEnabled<FrustumSystem>(), "This system can only be used with the FrustumSystem!");
		mCulledCount = 0;

		const auto& cameras = ecs.getView<FrustumComponent, CameraComponent>();

		std::vector<ECS::Entity> cameraViews;
		for (auto&& [cameraEntity, frustum, _] : cameras.each()) {
			cameraViews.push_back(static_cast<ECS::Entity>(std::numeric_limits<uint32_t>::max()));
		}

		auto bbView = ecs.getView<SpatialComponent, BoundingBoxComponent>();
		std::for_each(std::execution::seq, bbView.begin(), bbView.end(), [cameraViews, &cameras, &ecs, this](auto& entity) {
			std::vector<ECS::Entity> localCameraViews = cameraViews;

			int i = 0;
			for (auto&& [cameraEntity, frustum, _] : cameras.each()) {
				if (frustum.isInFrustum(*ecs.getComponent<SpatialComponent>(entity), *ecs.getComponent<BoundingBoxComponent>(entity))) {
					localCameraViews[i++] = cameraEntity;
				}
				else {
					mCulledCount++;
				}
			}

			if (auto* existingComp = ecs.getComponent<CameraCulledComponent>(entity)) {
				existingComp->mCameraViews.swap(localCameraViews);
			}
			else {
				ecs.addComponent<CameraCulledComponent>(entity, localCameraViews);
			}
		});
	}

	void FrustumCullingSystem::imguiEditor(ECS&) {
		ImGui::Text("Culled draws: %d", mCulledCount);
	}
}