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
		// One per job-system thread. Padded to a cache line because it is written on most iterations:
		// two workers incrementing counters that share a line would bounce it between their L1s
		// continuously, which would cost more than the culling test it is counting.
		//
		// C4324 reports that the alignment padded the structure, which is the entire point of it.
#pragma warning(push)
#pragma warning(disable : 4324)
		struct alignas(64) CulledCount {
			int mValue = 0;
		};
#pragma warning(pop)
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

		// Not a statistic - this call exists for its side effect, which is that it assures the pool.
		// try_get creates a missing pool on first sight, and creating one mutates the registry's pool
		// map; several workers doing that at once is a race. It would only ever fire on the frame a
		// scene's first culled entity appears, which is exactly the kind of bug that survives testing.
		// Done here, on main, where there is only one of us.
		const uint32_t alreadyCulledCount = ecs.entityCount<CameraCulledComponent>();
		NEO_UNUSED(alreadyCulledCount);

		JobSystem& jobSystem = ServiceLocator<JobSystem>::ref();
		std::vector<CulledCount> culled(jobSystem.numThreads());

		// Every frustum is read by every worker and written by none - FrustumSystem has already run and
		// nothing else is live while this system updates - so the cameras are shared freely. The pass
		// touches only its own entity's spatial, which matters because isInFrustum takes it by const
		// reference and the model-matrix getter still resolves lazily behind that.
		ecs.parallelForEach<SpatialComponent, BoundingBoxComponent>(
			[&ecs, &cameras, &culled, &jobSystem](ECS::Entity entity, const SpatialComponent& spatial, const BoundingBoxComponent& bb) {
				CulledCount& culledCount = culled[jobSystem.threadIndex()];

				// Per iteration now rather than hoisted out of the loop: a reused buffer would have to
				// be per thread to be safe, and only the first `count` entries are ever read anyway.
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
					// An entity reaches this once, on the frame it first appears. Safe from a worker -
					// the add stages into a mutex-guarded queue and the per-type registry behind it is
					// guarded too - and it stays here rather than being deferred to main precisely
					// because it is the path that is almost never taken.
					ecs.addComponent<CameraCulledComponent>(entity, cameraIDs, count);
				}
			});

		// Summed in thread order. Integer addition would give the same total in any order, but fixing
		// the order costs nothing and means the next reduction to be written here has an example that
		// is already right.
		mCulledCount = 0;
		for (const CulledCount& count : culled) {
			mCulledCount += count.mValue;
		}
	}

	void FrustumCullingSystem::imguiEditor(ECS&) {
		ImGui::Text("Culled draws: %d", mCulledCount);
	}
}
