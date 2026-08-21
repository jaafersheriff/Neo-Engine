#include "ECS/pch.hpp"
#include "FrustumCullingSystem.hpp"

#include "ECS/ECS.hpp"
#include "ECS/Component/CameraComponent/MainCameraComponent.hpp"
#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/CameraComponent/FrustumComponent.hpp"
#include "ECS/Component/CollisionComponent/CameraCulledComponent.hpp"

#include "ECS/Systems/CameraSystems/FrustumSystem.hpp"


namespace neo {

	namespace {
		struct CulledCount {
			int mValue = 0;
		};
	}

	void FrustumCullingSystem::update(ECS& ecs, const ResourceManagers& resourceManagers) {
		NEO_UNUSED(resourceManagers);

		TRACY_ZONEN("FrustumCullingSystem");
		NEO_ASSERT(ecs.isSystemEnabled<FrustumSystem>(), "This system can only be used with the FrustumSystem!");

		const auto& cameras = ecs.getView<FrustumComponent, CameraComponent>();
		const uint32_t cameraCount = ecs.entityCount<FrustumComponent>();
		NEO_ASSERT(cameraCount <= CameraCulledComponent::kMaxCameras,
			"Scene has %d frustums but CameraCulledComponent only tracks %d - the excess will never cull",
			cameraCount, CameraCulledComponent::kMaxCameras);

		// TODO - Called from the main thread for the side effect from ECS::ComponentRegistry::_ensure :(
		const uint32_t alreadyCulledCount = ecs.entityCount<CameraCulledComponent>();
		NEO_UNUSED(alreadyCulledCount);

		JobSystem& jobSystem = ServiceLocator<JobSystem>::ref();
		std::vector<CulledCount> culled(jobSystem.numThreads());

		ecs.parallelForEach<SpatialComponent, BoundingBoxComponent>(
			[&ecs, &cameras, &culled, &jobSystem](ECS::Entity entity, const SpatialComponent& spatial, const BoundingBoxComponent& bb) {
				CulledCount& culledCount = culled[jobSystem.threadIndex()];

				CameraCulledComponent::CameraIDs cameraIDs = {};
				uint8_t count = 0;
				for (auto&& [cameraEntity, frustum, _] : cameras.each()) {
					if (frustum.isInFrustum(spatial, bb)) {
						if (count < CameraCulledComponent::kMaxCameras) {
							cameraIDs[count++] = cameraEntity;
						}
					}
					else {
						culledCount.mValue++;
					}
				}

				if (auto* existingComp = ecs.getComponent<CameraCulledComponent>(entity)) {
					existingComp->mCameraIDs = cameraIDs;
					existingComp->mCameraCount = count;
				}
				else {
					ecs.addComponent<CameraCulledComponent>(entity, cameraIDs, count);
				}
			});

		mCulledCount = 0;
		for (const CulledCount& count : culled) {
			mCulledCount += count.mValue;
		}
	}

	void FrustumCullingSystem::imguiEditor(ECS&) {
		ImGui::Text("Culled draws: %d", mCulledCount);
	}
}
