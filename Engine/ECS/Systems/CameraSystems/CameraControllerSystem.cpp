#include "Engine/Engine.hpp"
#include "CameraControllerSystem.hpp"

#include "ECS/Component/EngineComponents/FrameStatsComponent.hpp"
#include "ECS/Component/HardwareComponent/MouseComponent.hpp"
#include "ECS/Component/HardwareComponent/KeyboardComponent.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"

#include "Util/Math.hpp"

namespace neo {

    void CameraControllerSystem::update(ECS& ecs) {
        for (auto&& [entity, controller, spatial] : ecs.getView<CameraControllerComponent, SpatialComponent>().each()) {
            if (auto frameStatsOpt = ecs.getComponent<FrameStatsComponent>()) {
                auto&& [_, frameStats] = *frameStatsOpt;
                _updateLook(frameStats.mDT, ecs, controller, spatial);
                _updatePosition(frameStats.mDT, ecs, controller, spatial);
            }
        }
    }

    void CameraControllerSystem::imguiEditor(ECS& ecs) {
        NEO_UNUSED(ecs);
        ImGui::SliderFloat("SuperSpeed", &mSuperSpeed, 1.f, 10.f);
    }

    void CameraControllerSystem::_updateLook(const float dt, ECS& ecs, CameraControllerComponent& controller, SpatialComponent& spatial) {
        if (auto mouseOpt = ecs.getComponent<MouseComponent>()) {
            auto&& [_, mouse] = *mouseOpt;
            glm::vec2 mousePos = mouse.mFrameMouse.getPos();
            glm::vec2 mouseSpeed = mouse.mFrameMouse.getSpeed();

            if (mouse.mFrameMouse.isDown(GLFW_MOUSE_BUTTON_2)) {
                controller.mTheta -= mouseSpeed.x * controller.mLookSpeed * dt;
                controller.mPhi -= mouseSpeed.y * controller.mLookSpeed * dt;
            }
        }

        if (auto keyboardOpt = ecs.getComponent<KeyboardComponent>()) {
            auto&& [_, keyboard] = *keyboardOpt;
            if (keyboard.mFrameKeyboard.isKeyPressed(controller.mLookLeftButton)) {
                controller.mTheta += controller.mLookSpeed * 2.f * dt;
            }
            if (keyboard.mFrameKeyboard.isKeyPressed(controller.mLookRightButton)) {
                controller.mTheta -= controller.mLookSpeed * 2.f * dt;
            }
            if (keyboard.mFrameKeyboard.isKeyPressed(controller.mLookUpButton)) {
                controller.mPhi -= controller.mLookSpeed * 2.f * dt;
            }
            if (keyboard.mFrameKeyboard.isKeyPressed(controller.mLookDownButton)) {
                controller.mPhi += controller.mLookSpeed * 2.f * dt;
            }
        }

        if (controller.mTheta > math::PI) {
            controller.mTheta = std::fmod(controller.mTheta, math::PI) - math::PI;
        }
        else if (controller.mTheta < -math::PI) {
            controller.mTheta = math::PI - std::fmod(-controller.mTheta, math::PI);
        }

        /* controller.mPhi [0.f, pi] */
        controller.mPhi = glm::max(glm::min(controller.mPhi, math::PI), 0.f);

        glm::vec3 w(-math::sphericalToCartesian(1.0f, controller.mTheta, controller.mPhi));
        w = glm::vec3(-w.y, w.z, -w.x); // one of the many reasons I like z to be up
        glm::vec3 v(math::sphericalToCartesian(1.0f, controller.mTheta, controller.mPhi - math::PI * 0.5f));
        v = glm::vec3(-v.y, v.z, -v.x);
        glm::vec3 u(glm::cross(v, w));

        spatial.setUVW(u, v, w);
    }

    void CameraControllerSystem::_updatePosition(const float dt, ECS& ecs, CameraControllerComponent& comp, SpatialComponent& spatial) {

        if (auto keyboardOpt = ecs.getComponent<KeyboardComponent>()) {
            auto&& [_, keyboard] = *keyboardOpt;
            int forward(keyboard.mFrameKeyboard.isKeyPressed(comp.mForwardButton));
            int backward(keyboard.mFrameKeyboard.isKeyPressed(comp.mBackwardButton));
            int right(keyboard.mFrameKeyboard.isKeyPressed(comp.mRightButton));
            int left(keyboard.mFrameKeyboard.isKeyPressed(comp.mLeftButton));
            int up(keyboard.mFrameKeyboard.isKeyPressed(comp.mUpButton));
            int down(keyboard.mFrameKeyboard.isKeyPressed(comp.mDownButton));
            bool speed(keyboard.mFrameKeyboard.isKeyPressed(GLFW_KEY_LEFT_SHIFT));

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
