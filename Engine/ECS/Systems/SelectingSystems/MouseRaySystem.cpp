#include "MouseRaySystem.hpp"

#include "ECS/ECS.hpp"
#include "ECS/Component/CameraComponent/MainCameraComponent.hpp"
#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/HardwareComponent/MouseComponent.hpp"
#include "ECS/Component/HardwareComponent/ViewportDetailsComponent.hpp"
#include "ECS/Component/RenderableComponent/LineMeshComponent.hpp"
#include "ECS/Component/SelectingComponent/MouseRayComponent.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"

namespace neo {

    void MouseRaySystem::update(ECS& ecs) {
        auto mainCamera = ecs.getView<MainCameraComponent, SpatialComponent>();
        NEO_ASSERT(mainCamera.size_hint() == 1, "Main camera doesn't exist");
        auto camera = ecs.getComponentAs<CameraComponent, PerspectiveCameraComponent>(mainCamera.front());

        auto viewport = ecs.getComponent<ViewportDetailsComponent>();
        NEO_ASSERT(viewport, "Window details don't exist");

        auto mouseRayView = ecs.getView<MouseComponent>();
        NEO_ASSERT(mouseRayView.size() == 1, "Can't be having more than one mouse");
        auto mouseRay = mouseRayView.front();
        auto mouseRayComp = ecs.getComponent<MouseRayComponent>();
        if (auto mouseOpt = ecs.getComponent<MouseComponent>()) {
            auto&& [_, mouse] = *mouseOpt;
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
                glm::vec3 dir = glm::normalize(glm::vec3(glm::inverse(ecs.getComponent<SpatialComponent>(mainCamera.front())->getView()) * mouseCoordsEye));
                glm::vec3 pos = ecs.getComponent<SpatialComponent>(mainCamera.front())->getPosition();

                // Create new mouseray if one doesnt exist
                if (!mouseRayComp) {
                    mouseRay = ecs.createEntity();
                    mouseRayComp = ecs.addComponent<MouseRayComponent>(mouseRay);
                }
                mouseRayComp->mDirection = dir;
                mouseRayComp->mPosition = pos;

                if (mShowRay) {
                    LineMeshComponent* line = ecs.getComponent<LineMeshComponent>(mouseRay);
                    if (!line) {
                        line = ecs.addComponent<LineMeshComponent>(mouseRay);
                    }
                    line->clearNodes();
                    line->addNodes({ {pos, glm::vec3(1.f)}, {pos + dir * camera->getNearFar().y, glm::vec3(1.f)} });
                }
            }
            else if (mouseRayComp && !mShowRay) {
                ecs.removeEntity(mouseRay);
            }
        }
    }
}
