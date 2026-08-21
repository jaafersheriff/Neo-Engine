#include "ECS/pch.hpp"
#include "SelectingSystem.hpp"

#include "ECS/ECS.hpp"
#include "ECS/Component/CollisionComponent/BoundingBoxComponent.hpp"
#include "ECS/Component/CollisionComponent/MouseRayComponent.hpp"
#include "ECS/Component/CollisionComponent/SelectedComponent.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"

namespace neo {

	namespace {
		struct alignas(64) Collision {
			ECS::Entity mEntity = ECS::NEO_INVALID_ENTITY;
			float mCollisionDistance = FLT_MAX;
		};

		// Nearest wins, lowest entity identifier breaks a tie
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
