#include "ECS/pch.hpp"
#include "MouseRaySystem.hpp"

#include "ECS/ECS.hpp"
#include "ECS/Component/CameraComponent/MainCameraComponent.hpp"
#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/CollisionComponent/MouseRayComponent.hpp"
#include "ECS/Component/HardwareComponent/MouseComponent.hpp"
#include "ECS/Component/HardwareComponent/ViewportDetailsComponent.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"

#include "GLFW/glfw3.h"

namespace neo {

	void MouseRaySystem::update(ECS& ecs, const ResourceManagers& resourceManagers) {
		NEO_UNUSED(resourceManagers);

		TRACY_ZONEN("MouseRaySystem");
		auto cameraTuple = ecs.getSingleView<MainCameraComponent, CameraComponent, SpatialComponent>();
		if (!cameraTuple) {
			return;
		}

		auto&& [cameraEntity, _, camera, cameraSpatial] = *ecs.getSingleView<MainCameraComponent, CameraComponent, SpatialComponent>();
		auto viewport = ecs.getComponent<ViewportDetailsComponent>();
		auto mouseRayComponent = ecs.getComponent<MouseRayComponent>();
		auto mouseComponent = ecs.getComponent<MouseComponent>();

		// Early exit
		if (!viewport.has_value() || !mouseComponent.has_value()) {
			if (mouseRayComponent.has_value()) {
				ecs.removeEntity(std::get<0>(*mouseRayComponent));
			}
			return;
		}

		auto&& [mouseEntity, mouse] = *mouseComponent;
		if (mouse.mFrameMouse.isDown(GLFW_MOUSE_BUTTON_1)) {
			// Mouse coords in viewport space
			glm::vec2 mouseCoords = mouse.mFrameMouse.getPos();

			// Mouse coords in NDC space
			auto framesize = std::get<1>(*viewport).mSize;
			mouseCoords = glm::vec2((2.f * mouseCoords.x) / framesize.x - 1.f, (2.f * mouseCoords.y) / framesize.y - 1.f);

			// Mouse coords in clip space to eye space
			glm::vec4 mouseCoordsEye = glm::inverse(camera.getProj()) * glm::vec4(mouseCoords, -1.f, 1.f);
			mouseCoordsEye.z = -1.f;
			mouseCoordsEye.w = 0.f;

			// Eye space to world space
			glm::vec3 dir = glm::normalize(glm::vec3(glm::inverse(ecs.getComponent<SpatialComponent>(cameraEntity)->getView()) * mouseCoordsEye));
			glm::vec3 pos = cameraSpatial.getPosition();

			// Create new mouseray if one doesnt exist
			ECS::Entity mouseRayEntity = entt::null;
			MouseRayComponent mouseRay;
			mouseRay.mDirection = dir;
			mouseRay.mPosition = pos;
			if (mouseRayComponent.has_value()) {
				mouseRayEntity = std::get<0>(*mouseRayComponent);
				std::get<1>(*mouseRayComponent) = mouseRay;
			}
			else {
				ecs.submitEntity(std::move(ECS::EntityBuilder{}
					.attachComponent<MouseRayComponent>(mouseRay)
				));
			}

		}
		else if (mouseRayComponent.has_value()) {
			ecs.removeEntity(std::get<0>(*mouseRayComponent));
		}
	}
}
