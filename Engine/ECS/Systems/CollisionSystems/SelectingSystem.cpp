#include "ECS/pch.hpp"
#include "SelectingSystem.hpp"

#include "ECS/ECS.hpp"
#include "ECS/Component/CollisionComponent/BoundingBoxComponent.hpp"
#include "ECS/Component/CollisionComponent/MouseRayComponent.hpp"
#include "ECS/Component/CollisionComponent/SelectedComponent.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"

namespace neo {

	namespace {
		// One per job-system thread. Padded to a cache line: a worker rewrites its nearest hit every
		// time it improves on it, and neighbouring entries sharing a line would bounce between L1s.
		//
		// C4324 reports that the alignment padded the structure, which is the entire point of it.
#pragma warning(push)
#pragma warning(disable : 4324)
		struct alignas(64) Collision {
			ECS::Entity mEntity = ECS::NEO_INVALID_ENTITY;
			float mCollisionDistance = FLT_MAX;
		};
#pragma warning(pop)

		// Nearest wins, lowest entity identifier breaks a tie. The tie-break is what makes the answer
		// independent of how the work was partitioned: without it, two entities at exactly the same
		// distance resolve to whichever one happened to be compared first, and that follows worker
		// timing rather than the scene. Applied to the per-thread accumulation as well as the combine,
		// since both are places where two candidates meet - a tie-break in only one of them still
		// leaves the result depending on which batch a candidate landed in.
		//
		// It does depart from the serial version, which kept whichever of the two it saw first in view
		// order. An exact tie needs coincident geometry to happen at all, and "lowest identifier" is at
		// least a rule that holds still between runs.
		bool isCloser(ECS::Entity entity, float distance, const Collision& best) {
			return distance < best.mCollisionDistance
				|| (distance == best.mCollisionDistance && entity < best.mEntity);
		}
	}

	void SelectingSystem::update(ECS& ecs, const ResourceManagers& resourceManagers) {
		NEO_UNUSED(resourceManagers);

		TRACY_ZONEN("SelectingSystem");
		auto mouseRayComponent = ecs.getComponent<MouseRayComponent>();
		auto selectedComponent = ecs.getComponent<SelectedComponent>();

		if (!mouseRayComponent.has_value()) {
			return;
		}

		// Copied out rather than captured. A structured binding cannot be captured by a lambda in
		// C++17, and the ray is two vectors that every worker reads and none writes.
		const glm::vec3 rayPosition = std::get<1>(*mouseRayComponent).mPosition;
		const glm::vec3 rayDirection = std::get<1>(*mouseRayComponent).mDirection;

		JobSystem& jobSystem = ServiceLocator<JobSystem>::ref();
		std::vector<Collision> hits(jobSystem.numThreads());

		ecs.parallelForEach<BoundingBoxComponent, SpatialComponent>(
			[&hits, &jobSystem, rayPosition, rayDirection](ECS::Entity entity, const BoundingBoxComponent& bb, const SpatialComponent& spatial) {
				// Ignore static entities
				if (bb.mStatic) {
					return;
				}

				const auto intersection = bb.intersect(spatial.getModelMatrix(), rayPosition, rayDirection);
				if (!intersection.has_value()) {
					return;
				}

				Collision& best = hits[jobSystem.threadIndex()];
				if (isCloser(entity, intersection.value(), best)) {
					best.mEntity = entity;
					best.mCollisionDistance = intersection.value();
				}
			});

		// An untouched bucket is FLT_MAX against the invalid entity, whose identifier is all bits set,
		// so it can neither win outright nor win a tie against a real hit.
		Collision collision;
		for (const Collision& hit : hits) {
			if (isCloser(hit.mEntity, hit.mCollisionDistance, collision)) {
				collision = hit;
			}
		}

		if (collision.mCollisionDistance < FLT_MAX) {
			if (selectedComponent.has_value()) {
				if (std::get<0>(*selectedComponent) != collision.mEntity) {
					ecs.removeComponent<SelectedComponent>(std::get<0>(*selectedComponent));
					ecs.addComponent<SelectedComponent>(collision.mEntity);
				}
			}
			else {
				ecs.addComponent<SelectedComponent>(collision.mEntity);
			}
		}
		else if (selectedComponent.has_value()) {
			ecs.removeComponent<SelectedComponent>(std::get<0>(*selectedComponent));
		}
	}
}
