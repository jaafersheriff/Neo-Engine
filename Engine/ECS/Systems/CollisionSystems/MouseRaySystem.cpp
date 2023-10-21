#include "ECS/pch.hpp"
#include "MouseRaySystem.hpp"

#include "ECS/ECS.hpp"
#include "ECS/Component/CameraComponent/MainCameraComponent.hpp"
#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/CollisionComponent/MouseRayComponent.hpp"
#include "ECS/Component/HardwareComponent/MouseComponent.hpp"
#include "ECS/Component/HardwareComponent/ViewportDetailsComponent.hpp"
#include "ECS/Component/RenderableComponent/LineMeshComponent.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"

#include "GLFW/glfw3.h"

namespace neo {

    void MouseRaySystem::update(ECS& ecs) {
        TRACY_ZONEN("MouseRaySystem");
        auto&& [cameraEntity, _, cameraSpatial] = *ecs.getSingleView<MainCameraComponent, SpatialComponent>();
        auto camera = ecs.getComponentAs<CameraComponent, PerspectiveCameraComponent>(cameraEntity);

        auto viewport = ecs.getComponent<ViewportDetailsComponent>();
        NEO_ASSERT(viewport, "Window details don't exist");

        auto mouseRayView = ecs.getComponent<MouseRayComponent>();
        if (auto mouseOpt = ecs.getComponent<MouseComponent>()) {
            auto&& [__, mouse] = *mouseOpt;
            if (mouse.mFrameMouse.isDown(GLFW_MOUSE_BUTTON_1)) {
                // Mouse coords in viewport space
                glm::vec2 mouseCoords = mouse.mFrameMouse.getPos();

                // Mouse coords in NDC space
                auto framesize = std::get<1>(*viewport).mSize;
                mouseCoords = glm::vec2((2.f * mouseCoords.x) / framesize.x - 1.f, (2.f * mouseCoords.y) / framesize.y - 1.f);

                // Mouse coords in clip space to eye space
                glm::vec4 mouseCoordsEye = glm::inverse(camera->getProj()) * glm::vec4(mouseCoords, -1.f, 1.f);
                mouseCoordsEye.z = -1.f;
                mouseCoordsEye.w = 0.f;

                // Eye space to world space
                glm::vec3 dir = glm::normalize(glm::vec3(glm::inverse(ecs.getComponent<SpatialComponent>(cameraEntity)->getView()) * mouseCoordsEye));
                glm::vec3 pos = cameraSpatial.getPosition();

                // Create new mouseray if one doesnt exist
                ECS::Entity mouseRayEntity = entt::null;
                MouseRayComponent* mouseRay;
                if (mouseRayView) {
                    mouseRayEntity = std::get<0>(*mouseRayView);
                    mouseRay = &std::get<1>(*mouseRayView);
                }
                else {
                    mouseRayEntity = ecs.createEntity();
                    mouseRay = ecs.addComponent<MouseRayComponent>(mouseRayEntity);
                }
                mouseRay->mDirection = dir;
                mouseRay->mPosition = pos;

                if (mShowRay) {
                    LineMeshComponent* line = ecs.getComponent<LineMeshComponent>(mouseRayEntity);
                    if (!line) {
                        line = ecs.addComponent<LineMeshComponent>(mouseRayEntity);
                    }
                    line->clearNodes();
                    line->addNodes({ {pos, glm::vec3(1.f)}, {pos + dir * camera->getNearFar().y, glm::vec3(1.f)} });
                }
            }
            else if (mouseRayView) {
                if (!mShowRay) {
                    ecs.removeEntity(std::get<0>(*mouseRayView));
                }
                else {
                    ecs.removeComponent<MouseRayComponent>(std::get<0>(*mouseRayView));
                }
            }
        }
    }
}
