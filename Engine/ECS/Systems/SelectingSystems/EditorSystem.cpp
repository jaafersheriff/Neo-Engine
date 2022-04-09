#include "EditorSystem.hpp"
#include "Engine/Engine.hpp"

#include "ECS/Component/CameraComponent/MainCameraComponent.hpp"
#include "ECS/Component/CameraComponent/FrustumComponent.hpp"
#include "ECS/Component/CollisionComponent/BoundingBoxComponent.hpp"
#include "ECS/Component/HardwareComponent/MouseComponent.hpp"
#include "ECS/Component/RenderableComponent/OutlineRenderable.hpp"
#include "ECS/Component/SelectingComponent/MouseRayComponent.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"

namespace neo {

	EditorSystem::EditorSystem()
		: SelectingSystem(
			"Editor System",
			[](ECS& ecs, ECS::Entity entity) {
				ecs.removeComponent<renderable::OutlineRenderable>(entity);
			},
			[](ECS& ecs, ECS::Entity entity) {
				if (!ecs.has<renderable::OutlineRenderable>(entity)) {
					ecs.addComponent<renderable::OutlineRenderable>(entity, glm::vec4(1.f, 0.95f, 0.72f, 1.f), 3.f);
				}
			},
			[](ECS& ecs, ECS::Entity entity) {
				NEO_UNUSED(ecs, entity);
			}
		)
	{}

	// TODO : add hovered capability
	void EditorSystem::update(ECS& ecs) {
		NEO_UNUSED(ecs);
		// if (auto camera = ecs.getSingleView<MainCameraComponent, FrustumComponent>()) {
		// 	auto&& [_, __, cameraFrustum] = *camera;
		// 	if (auto mouseRayOpt = ecs.getComponent<MouseRayComponent>()) {
		// 		auto&& [___, mouseRay] = *mouseRayOpt;
		// 		if (auto mouseOpt = ecs.getComponent<MouseComponent>()) { 
		// 			auto&& [____, mouse] = *mouseOpt;
		// 			if (auto selected = ecs.getSingleView<SelectedComponent, SpatialComponent>()) {
		// 				auto&& [selectedEntity, un4, selectedSpatial] = *selected;
		// 				glm::vec3 pos;
		// 				if (auto bb = ecs.getComponent<BoundingBoxComponent>(selectedEntity)) {
		// 					if (cameraFrustum.isInFrustum(selectedSpatial, *bb)) {
		// 						glm::vec3 worldSpaceCenter = selectedSpatial.getModelMatrix() * glm::vec4(bb->getCenter(), 1.f);
		// 						glm::vec3 offsetTranslation = selectedSpatial.getPosition() - worldSpaceCenter;
		// 						float distance = glm::distance(worldSpaceCenter, mouseRay.mPosition);
		// 						distance += mouse.mFrameMouse.getScrollSpeed();
		// 						pos = mouseRay.mPosition + mouseRay.mDirection * distance + offsetTranslation;
		// 					}
		// 				}
		// 				else {
		// 					float distance = glm::distance(selectedSpatial.getPosition(), mouseRay.mPosition);
		// 					distance += mouse.mFrameMouse.getScrollSpeed();
		// 					pos = mouseRay.mPosition + mouseRay.mDirection * distance;
		// 				}
		// 				selectedSpatial.setPosition(pos);
		// 			}
		// 		}
		// 	}
		// }
	}
}
