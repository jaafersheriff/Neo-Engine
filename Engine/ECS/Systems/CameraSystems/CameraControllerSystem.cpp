#include "Engine/Engine.hpp"
#include "CameraControllerSystem.hpp"

#include "ECS/Component/EngineComponents/FrameStatsComponent.hpp"
#include "ECS/Component/HardwareComponent/MouseComponent.hpp"
#include "ECS/Component/HardwareComponent/KeyboardComponent.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"

namespace neo {

    void CameraControllerSystem::update(ECS& ecs) {
        for (auto&& [entity, controller, spatial] : ecs.getView<CameraControllerComponent, SpatialComponent>().each()) {
            if (auto frameStats = ecs.getComponent<FrameStatsComponent>()) {
                _updateLook(frameStats->mDT, ecs, controller, spatial);
                _updatePosition(frameStats->mDT, ecs, controller, spatial);
            }
        }
    }

    void CameraControllerSystem::imguiEditor(ECS& ecs) {
        NEO_UNUSED(ecs);
        ImGui::SliderFloat("SuperSpeed", &mSuperSpeed, 1.f, 10.f);
    }

    void CameraControllerSystem::_updateLook(const float dt, ECS& ecs, CameraControllerComponent& controller, SpatialComponent& spatial) {
        float theta = 0.f;
        float phi = util::PI * 0.5f;

        if (auto mouse = ecs.getComponent<MouseComponent>()) {
            glm::vec2 mousePos = mouse->mFrameMouse.getPos();
            glm::vec2 mouseSpeed = mouse->mFrameMouse.getSpeed();

            if (mouse->mFrameMouse.isDown(GLFW_MOUSE_BUTTON_2)) {
                theta -= mouseSpeed.x * controller.mLookSpeed * dt;
                phi -= mouseSpeed.y * controller.mLookSpeed * dt;
            }
        }

        if (auto keyboard = ecs.getComponent<KeyboardComponent>()) {
            if (keyboard->mFrameKeyboard.isKeyPressed(controller.mLookLeftButton)) {
                theta += controller.mLookSpeed * 2.f * dt;
            }
            if (keyboard->mFrameKeyboard.isKeyPressed(controller.mLookRightButton)) {
                theta -= controller.mLookSpeed * 2.f * dt;
            }
            if (keyboard->mFrameKeyboard.isKeyPressed(controller.mLookUpButton)) {
                phi -= controller.mLookSpeed * 2.f * dt;
            }
            if (keyboard->mFrameKeyboard.isKeyPressed(controller.mLookDownButton)) {
                phi += controller.mLookSpeed * 2.f * dt;
            }
        }

        if (theta > util::PI) {
            theta = std::fmod(theta, util::PI) - util::PI;
        }
        else if (theta < -util::PI) {
            theta = util::PI - std::fmod(-theta, util::PI);
        }

        /* phi [0.f, pi] */
        phi = glm::max(glm::min(phi, util::PI), 0.f);

        glm::vec3 w(-util::sphericalToCartesian(1.0f, theta, phi));
        w = glm::vec3(-w.y, w.z, -w.x); // one of the many reasons I like z to be up
        glm::vec3 v(util::sphericalToCartesian(1.0f, theta, phi - util::PI * 0.5f));
        v = glm::vec3(-v.y, v.z, -v.x);
        glm::vec3 u(glm::cross(v, w));

        spatial.setUVW(u, v, w);
    }

    void CameraControllerSystem::_updatePosition(const float dt, ECS& ecs, CameraControllerComponent& comp, SpatialComponent& spatial) {

        if (auto keyboard = ecs.getComponent<KeyboardComponent>()) {
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
                dir = glm::normalize(dir);
                dir = glm::normalize(
                        spatial.getRightDir() * dir.x +
                        spatial.getUpDir()    * dir.y +
                    -spatial.getLookDir()     * dir.z);
                spatial.move(dir * comp.mMoveSpeed * dt * (speed ? mSuperSpeed : 1.f));
            }
        }
    }
}
