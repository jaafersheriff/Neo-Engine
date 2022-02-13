#include "CameraControllerComponent.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"

#include "ECS/GameObject.hpp"

#include "Util/Util.hpp"

#include <imgui/imgui.h>
#include <GLFW/glfw3.h>
#include "microprofile.h"

namespace neo {

    CameraControllerComponent::CameraControllerComponent(GameObject *go, float lookSpeed, float moveSpeed) :
        Component(go),
        mTheta(0.f),
        mPhi(util::PI * 0.5f),
        mLookSpeed(lookSpeed),
        mMoveSpeed(moveSpeed),
        mLookUpButton(GLFW_KEY_UP),
        mLookDownButton(GLFW_KEY_DOWN),
        mLookRightButton(GLFW_KEY_RIGHT),
        mLookLeftButton(GLFW_KEY_LEFT),
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
        MICROPROFILE_SCOPEI("CameraControllerComponent", "_updateSpatialOrientation", MP_AUTO);
        if (mTheta > util::PI) {
            mTheta = std::fmod(mTheta, util::PI) - util::PI;
        }
        else if (mTheta < -util::PI) {
            mTheta = util::PI - std::fmod(-mTheta, util::PI);
        }

        /* phi [0.f, pi] */
        mPhi = glm::max(glm::min(mPhi, util::PI), 0.f);

        glm::vec3 w(-util::sphericalToCartesian(1.0f, mTheta, mPhi));
        w = glm::vec3(-w.y, w.z, -w.x); // one of the many reasons I like z to be up
        glm::vec3 v(util::sphericalToCartesian(1.0f, mTheta, mPhi - util::PI * 0.5f));
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