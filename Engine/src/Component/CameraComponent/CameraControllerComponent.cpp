#include "CameraControllerComponent.hpp"
#include "Component/SpatialComponent/SpatialComponent.hpp"

#include "GameObject/GameObject.hpp"

#include "Window/Mouse.hpp"
#include "Window/Keyboard.hpp"

#include "Util/Util.hpp"

namespace neo {

    CameraControllerComponent::CameraControllerComponent(GameObject *go, float lookSpeed, float moveSpeed) :
        Component(go),
        mTheta(0.f),
        mPhi(Util::PI() * 0.5f),
        mLookSpeed(lookSpeed),
        mMoveSpeed(moveSpeed) {
        setButtons(GLFW_KEY_W, GLFW_KEY_S, GLFW_KEY_A, GLFW_KEY_D, GLFW_KEY_E, GLFW_KEY_Q);
    }

    void CameraControllerComponent::setButtons(int f, int b, int l, int r, int u, int d) {
        mForwardButton  = f;
        mBackwardButton = b;
        mRightButton    = r;
        mLeftButton     = l;
        mUpButton       = u;
        mDownButton     = d;
    }

    void CameraControllerComponent::update(float dt) {
        glm::vec2 mousePos = Mouse::getPos();
        glm::vec2 mouseSpeed = Mouse::getSpeed();
        if (Mouse::isDown(GLFW_MOUSE_BUTTON_1) && (mousePos.x || mousePos.y)) {
            mTheta -= mouseSpeed.x * mLookSpeed * dt;
            mPhi   += mouseSpeed.y * mLookSpeed * dt;
            _updateSpatialOrientation();
        }

        int forward(Keyboard::isKeyPressed(mForwardButton));
        int backward(Keyboard::isKeyPressed(mBackwardButton));
        int right(Keyboard::isKeyPressed(mRightButton));
        int left(Keyboard::isKeyPressed(mLeftButton));
        int up(Keyboard::isKeyPressed(mUpButton));
        int down(Keyboard::isKeyPressed(mDownButton));

        glm::vec3 dir(
            float(right - left),
            float(up - down),
            float(backward - forward)
        );

        if (dir != glm::vec3()) {
            auto spatial = mGameObject->getSpatial();
            dir = glm::normalize(dir);
            dir = glm::normalize(
                spatial->getU() * dir.x + 
                glm::vec3(0.f, 1.f, 0.f) * dir.y + 
                spatial->getW() * dir.z);
            spatial->move(dir * mMoveSpeed * dt);
        }
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
        mGameObject->getSpatial()->setUVW(u, v, w);
    }
}