#include "ECS/pch.hpp"
#include "CameraControllerComponent.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"

#include "GLFW/glfw3.h"

namespace neo {

    CameraControllerComponent::CameraControllerComponent(float lookSpeed, float moveSpeed)
        : mLookSpeed(lookSpeed)
        , mMoveSpeed(moveSpeed)
        , mLookUpButton(GLFW_KEY_UP)
        , mLookDownButton(GLFW_KEY_DOWN)
        , mLookRightButton(GLFW_KEY_RIGHT)
        , mLookLeftButton(GLFW_KEY_LEFT)
        , mForwardButton(GLFW_KEY_W)
        , mBackwardButton(GLFW_KEY_S)
        , mRightButton(GLFW_KEY_D)
        , mLeftButton(GLFW_KEY_A)
        , mUpButton(GLFW_KEY_SPACE)
        , mDownButton(GLFW_KEY_LEFT_CONTROL) 
        , mTheta(0.f) 
        , mPhi(util::PI * 0.5f)
    { }

    void CameraControllerComponent::imGuiEditor() {
        ImGui::SliderFloat("Look speed", &mLookSpeed, 0.1f, 10.f);
        ImGui::SliderFloat("Move speed", &mMoveSpeed, 0.1f, 10.f);
    }
}