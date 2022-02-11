#include "Engine/Engine.hpp"
#include "CameraControllerSystem.hpp"

namespace neo {

    void CameraControllerSystem::update(const float dt) {
        if (auto comp = Engine::getSingleComponent<CameraControllerComponent>()) {
            _updateLook(dt, *comp);
            _updatePosition(dt, *comp);
        }
    }

    void CameraControllerSystem::imguiEditor() {
        ImGui::SliderFloat("SuperSpeed", &mSuperSpeed, 1.f, 10.f);
    }

    void CameraControllerSystem::_updateLook(const float dt, CameraControllerComponent& comp) {
        if (auto mouse = Engine::getSingleComponent<MouseComponent>()) {
            glm::vec2 mousePos = mouse->mFrameMouse.getPos();
            glm::vec2 mouseSpeed = mouse->mFrameMouse.getSpeed();

            if (mouse->mFrameMouse.isDown(GLFW_MOUSE_BUTTON_2) && (mousePos.x || mousePos.y)) {
                float theta = comp.mTheta - mouseSpeed.x * comp.mLookSpeed * dt;
                float phi = comp.mPhi + mouseSpeed.y * comp.mLookSpeed * dt;
                comp.setOrientation(theta, phi);
            }
        }


        if (auto keyboard = Engine::getSingleComponent<KeyboardComponent>()) {
            if (keyboard->mFrameKeyboard.isKeyPressed(comp.mLookLeftButton)) {
                float theta = comp.mTheta + comp.mLookSpeed * 2.f * dt;
                comp.setOrientation(theta, comp.mPhi);
            }
            if (keyboard->mFrameKeyboard.isKeyPressed(comp.mLookRightButton)) {
                float theta = comp.mTheta - comp.mLookSpeed * 2.f * dt;
                comp.setOrientation(theta, comp.mPhi);
            }
            if (keyboard->mFrameKeyboard.isKeyPressed(comp.mLookUpButton)) {
                float phi = comp.mPhi - comp.mLookSpeed * 2.f * dt;
                comp.setOrientation(comp.mTheta, phi);
            }
            if (keyboard->mFrameKeyboard.isKeyPressed(comp.mLookDownButton)) {
                float phi = comp.mPhi + comp.mLookSpeed * 2.f * dt;
                comp.setOrientation(comp.mTheta, phi);
            }
        }
    }

    void CameraControllerSystem::_updatePosition(const float dt, CameraControllerComponent& comp) {

        if (auto keyboard = Engine::getSingleComponent<KeyboardComponent>()) {
            int forward(keyboard->mFrameKeyboard.isKeyPressed(comp.mForwardButton));
            int backward(keyboard->mFrameKeyboard.isKeyPressed(comp.mBackwardButton));
            int right(keyboard->mFrameKeyboard.isKeyPressed(comp.mRightButton));
            int left(keyboard->mFrameKeyboard.isKeyPressed(comp.mLeftButton));
            int up(keyboard->mFrameKeyboard.isKeyPressed(comp.mUpButton));
            int down(keyboard->mFrameKeyboard.isKeyPressed(comp.mDownButton));
            bool speed(keyboard->mFrameKeyboard.isKeyPressed(GLFW_KEY_LEFT_SHIFT));

            glm::vec3 dir(
                float(right - left),
                float(up - down),
                float(backward - forward)
            );

            if (dir != glm::vec3()) {
                auto spatial = comp.getGameObject().getComponentByType<SpatialComponent>();
                NEO_ASSERT(spatial, "Camera doesn't have SpatialComponent");
                dir = glm::normalize(dir);
                dir = glm::normalize(
                    spatial->getRightDir() * dir.x +
                    spatial->getUpDir() * dir.y +
                    -spatial->getLookDir() * dir.z);
                spatial->move(dir * comp.mMoveSpeed * dt * (speed ? mSuperSpeed : 1.f));
            }
        }
    }
}
