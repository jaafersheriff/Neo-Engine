#include "ECS/pch.hpp"
#include "SelectingSystem.hpp"

#include "ECS/ECS.hpp"
#include "ECS/Component/CollisionComponent/BoundingBoxComponent.hpp"
#include "ECS/Component/CollisionComponent/MouseRayComponent.hpp"
#include "ECS/Component/CollisionComponent/SelectedComponent.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"

#include <ImGuizmo.h>

namespace neo {

	void SelectingSystem::update(ECS& ecs) {
		TRACY_ZONEN("SelectingSystem");
		auto mouseRayComponent = ecs.getComponent<MouseRayComponent>();
		auto selectedComponent = ecs.getComponent<SelectedComponent>();

		if (ImGuizmo::IsUsing() || !mouseRayComponent.has_value()) {
			return;
		}

		auto&& [_, mouseRay] = *mouseRayComponent;
		struct Collision {
			ECS::Entity mEntity;
			float mCollisionDistance = FLT_MAX;
		};
		auto selectables = ecs.getView<BoundingBoxComponent, SpatialComponent>();
		Collision collision;
		for (auto&& [entity, bb, spatial] : selectables.each()) {
			// Ignore static entities
			if (bb.mStatic) {
				continue;
			}
			auto intersection = bb.intersect(spatial.getModelMatrix(), mouseRay.mPosition, mouseRay.mDirection);
			if (intersection.has_value() && intersection.value() < collision.mCollisionDistance) {
				collision.mEntity = entity;
				collision.mCollisionDistance = intersection.value();
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
