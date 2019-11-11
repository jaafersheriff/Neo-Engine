#include "CameraControllerComponent.hpp"
#include "Component/SpatialComponent/SpatialComponent.hpp"

#include "GameObject/GameObject.hpp"

#include "Window/Mouse.hpp"
#include "Window/Keyboard.hpp"

#include "Util/Util.hpp"
#include "ext/imgui/imgui.h"

namespace neo {

    CameraControllerComponent::CameraControllerComponent(GameObject *go, float lookSpeed, float moveSpeed) :
        Component(go),
        mTheta(0.f),
        mPhi(Util::PI() * 0.5f),
        mLookSpeed(lookSpeed),
        mMoveSpeed(moveSpeed),
        mForwardButton(GLFW_KEY_W),
        mBackwardButton(GLFW_KEY_S),
        mRightButton(GLFW_KEY_D),
        mLeftButton(GLFW_KEY_A),
        mUpButton(GLFW_KEY_SPACE),
        mDownButton(GLFW_KEY_LEFT_CONTROL) {

    }

    void CameraControllerComponent::setOrientation(float theta, float phi) {
        this->mTheta = theta;
        this->mPhi = phi;
        _updateSpatialOrientation();
    }

    void CameraControllerComponent::_updateSpatialOrientation() {
        if (mTheta > Util::PI()) {
            mTheta = std::fmod(mTheta, Util::PI()) - Util::PI();
        }
        else if (mTheta < -Util::PI()) {
            mTheta = Util::PI() - std::fmod(-mTheta, Util::PI());
        }

        /* phi [0.f, pi] */
        mPhi = glm::max(glm::min(mPhi, Util::PI()), 0.f);

        glm::vec3 w(-Util::sphericalToCartesian(1.0f, mTheta, mPhi));
        w = glm::vec3(-w.y, w.z, -w.x); // one of the many reasons I like z to be up
        glm::vec3 v(Util::sphericalToCartesian(1.0f, mTheta, mPhi - Util::PI() * 0.5f));
        v = glm::vec3(-v.y, v.z, -v.x);
        glm::vec3 u(glm::cross(v, w));

        auto spatial = mGameObject->getComponentByType<SpatialComponent>();
        NEO_ASSERT(spatial, "CameraController has no SpatialComponent");
        spatial->setUVW(u, v, w);
    }

    void CameraControllerComponent::imGuiEditor() {
        ImGui::SliderFloat("Look speed", &mLookSpeed, 0.1f, 10.f);
        ImGui::SliderFloat("Move speed", &mMoveSpeed, 0.1f, 10.f);
    }
}