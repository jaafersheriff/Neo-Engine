#include <Engine.hpp>
#include "CameraControllerSystem.hpp"

namespace neo {

    void CameraControllerSystem::update(const float dt) {
        glm::vec2 mousePos = Mouse::getPos();
        glm::vec2 mouseSpeed = Mouse::getSpeed();

        for (auto comp : Engine::getComponents<CameraControllerComponent>()) {
            if (Mouse::isDown(GLFW_MOUSE_BUTTON_1) && (mousePos.x || mousePos.y)) {
                float theta = comp->mTheta - mouseSpeed.x * comp->mLookSpeed * dt;
                float phi = comp->mPhi + mouseSpeed.y * comp->mLookSpeed * dt;
                comp->setOrientation(theta, phi);
            }

            int forward(Keyboard::isKeyPressed(comp->mForwardButton));
            int backward(Keyboard::isKeyPressed(comp->mBackwardButton));
            int right(Keyboard::isKeyPressed(comp->mRightButton));
            int left(Keyboard::isKeyPressed(comp->mLeftButton));
            int up(Keyboard::isKeyPressed(comp->mUpButton));
            int down(Keyboard::isKeyPressed(comp->mDownButton));

            glm::vec3 dir(
                float(right - left),
                float(up - down),
                float(backward - forward)
            );

            if (dir != glm::vec3()) {
                auto spatial = comp->getGameObject().getSpatial();
                dir = glm::normalize(dir);
                dir = glm::normalize(
                    spatial->mU * dir.x +
                    glm::vec3(0.f, 1.f, 0.f) * dir.y +
                    spatial->mW * dir.z);
                comp->getGameObject().getSpatial()->move(dir * comp->mMoveSpeed * dt);
            }
        }
    }

}
