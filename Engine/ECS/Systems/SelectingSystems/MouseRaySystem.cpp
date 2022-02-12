#include "Engine/Engine.hpp"

#include "MouseRaySystem.hpp"

namespace neo {

    void MouseRaySystem::update(ECS& ecs) {
        auto mainCamera = ecs.getComponentTuple<MainCameraComponent, CameraComponent>();
        NEO_ASSERT(mainCamera, "Main camera doesn't exist");
        auto camera = mainCamera->get<CameraComponent>();

        auto windowDetails = ecs.getSingleComponent<WindowDetailsComponent>();
        NEO_ASSERT(windowDetails, "Window details don't exist");

        auto mouseRayComp = ecs.getSingleComponent<MouseRayComponent>();
        if (auto mouse = ecs.getSingleComponent<MouseComponent>()) {
            if (mouse->mFrameMouse.isDown(GLFW_MOUSE_BUTTON_1)) {
                // Mouse coords in viewport space
                glm::vec2 mouseCoords = mouse->mFrameMouse.getPos();

                // Mouse coords in NDC space
                auto framesize = windowDetails->mDetails.getSize();
                mouseCoords = glm::vec2((2.f * mouseCoords.x) / framesize.x - 1.f, 1.f - (2.f * mouseCoords.y) / framesize.y);

                // Mouse coords in clip space to eye space
                glm::vec4 mouseCoordsEye = glm::inverse(camera->getProj()) * glm::vec4(mouseCoords, -1.f, 1.f);
                mouseCoordsEye.z = -1.f;
                mouseCoordsEye.w = 0.f;

                // Eye space to world space
                glm::vec3 dir = glm::normalize(glm::vec3(glm::inverse(camera->getView()) * mouseCoordsEye));
                glm::vec3 pos = camera->getGameObject().getComponentByType<SpatialComponent>()->getPosition();

                // Create new mouseray if one doesnt exist
                if (!mouseRayComp) {
                    mouseRayComp = &ecs.addComponent<MouseRayComponent>(&ecs.createGameObject());
                }
                mouseRayComp->mDirection = dir;
                mouseRayComp->mPosition = pos;

                if (mShowRay) {
                    LineMeshComponent* line = mouseRayComp->getGameObject().getComponentByType<LineMeshComponent>();
                    if (!line) {
                        line = &ecs.addComponent<LineMeshComponent>(&mouseRayComp->getGameObject());
                    }
                    line->clearNodes();
                    line->addNodes({ {pos, glm::vec3(1.f)}, {pos + dir * camera->getNearFar().y, glm::vec3(1.f)} });
                }
            }
            else if (mouseRayComp && !mShowRay) {
                ecs.removeGameObject(mouseRayComp->getGameObject());
            }
        }
    }
}
