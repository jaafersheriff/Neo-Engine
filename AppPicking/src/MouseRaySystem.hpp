#pragma once

#include "Systems/System.hpp"
#include "Engine.hpp"

#include "MouseRayComponent.hpp"

using namespace neo;

class MouseRaySystem : public System {

public:
    MouseRaySystem() :
        System("MouseRay System")
    {}


    virtual void update(const float dt) override {
        auto camera = Engine::getSingleComponent<MainCameraComponent>();
        auto mouseRayComp = Engine::getSingleComponent<MouseRayComponent>();
        if (!camera || !camera->getGameObject().getComponentByType<CameraComponent>()) {
            return;
        }

        if (Mouse::isDown(GLFW_MOUSE_BUTTON_1)) {
            // Create new mouseray if one doesnt exist
            if (!mouseRayComp) {
                mouseRayComp = &Engine::addComponent<MouseRayComponent>(&Engine::createGameObject());
            }

            // Mouse coords in viewport space
            glm::vec2 mouseCoords = Mouse::getPos();

            // Mouse coords in NDC space
            mouseCoords = glm::vec2((2.f * mouseCoords.x) / Window::getSize().x - 1.f, 1.f - (2.f * mouseCoords.y) / Window::getSize().y);

            // Mouse coords in clip space to eye space
            glm::vec4 mouseCoordsEye = glm::inverse(camera->getGameObject().getComponentByType<CameraComponent>()->getProj()) * glm::vec4(mouseCoords, -1.f, 1.f);
            mouseCoordsEye.z = -1.f;
            mouseCoordsEye.w = 0.f;

            // Eye space to world space
            mouseRayComp->ray = glm::normalize(glm::vec3(glm::inverse(camera->getGameObject().getComponentByType<CameraComponent>()->getView()) * mouseCoordsEye));
            mouseRayComp->position = camera->getGameObject().getComponentByType<SpatialComponent>()->getPosition();
        }
        else {
            Engine::removeComponent<MouseRayComponent>(*mouseRayComp);
        }
    }
};
