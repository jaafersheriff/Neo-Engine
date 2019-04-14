#include "CameraComponent.hpp"
#include "Component/SpatialComponent/SpatialComponent.hpp"

#include "GameObject/GameObject.hpp"
#include "Messaging/Messenger.hpp"

#include "Window/Window.hpp"

#include "glm/gtc/matrix_transform.hpp"

namespace neo {

    CameraComponent::CameraComponent(GameObject *gameObject, float near, float far, bool isOrtho = false) :
        Component(gameObject),
        mNear(near),
        mFar(far),
        mFOV(0.f),
        mHorizBounds(),
        mVertBounds(),
        mViewMat(),
        mProjMat(),
        mViewMatDirty(true),
        mProjMatDirty(true),
        mIsOrtho(isOrtho)
    {}

    CameraComponent::CameraComponent(GameObject *gameObject, float fov, float near, float far) :
        CameraComponent(gameObject, near, far, false) {
        setFOV(fov);
    }

    CameraComponent::CameraComponent(GameObject *gameObject, float horizMin, float horizMax, float vertMin, float vertMax, float near, float far) 
        : CameraComponent(gameObject, near, far, true) {
        setOrthoBounds(glm::vec2(horizMin, horizMax), glm::vec2(vertMin, vertMax));
    }

    void CameraComponent::init() {
        mViewMatDirty = true;
        mProjMatDirty = true;

        Messenger::addReceiver<SpatialChangeMessage>(mGameObject, [&](const Message & msg_) {
            mViewMatDirty = true;
        });

        if (!mIsOrtho) {
            Messenger::addReceiver<WindowFrameSizeMessage>(nullptr, [&] (const Message & msg_) {
                mProjMatDirty = true;
            });
        }
    }

    void CameraComponent::setFOV(float fov) {
        if (fov == mFOV) {
            return;
        }

        mFOV = fov;
        mProjMatDirty = true;
    }

    void CameraComponent::setNearFar(float near, float far) {
        if (near == mNear && far == mFar) {
            return;
        }

        mNear = near;
        mFar = far;
        mProjMatDirty = true;
    }

    void CameraComponent::setOrthoBounds(const glm::vec2 &h, const glm::vec2 &v) {
        if (h == mHorizBounds && v == mVertBounds) {
            return;
        }

        mHorizBounds = h;
        mVertBounds = v;
        mProjMatDirty = true;
        mViewMatDirty = true;
    }

    void CameraComponent::setLookDir(glm::vec3 dir) {
        glm::vec3 w = -glm::normalize(dir);
        auto spatial = mGameObject->getSpatial();
        if (w == spatial->mW) {
            return;
        }
        glm::vec3 u = glm::cross(w, glm::vec3(0, 1, 0));
        glm::vec3 v = glm::cross(u, w);
        u = glm::cross(v, w);
        spatial->setUVW(u, v, w);
    }

    const glm::vec3 CameraComponent::getLookDir() const {
        return -mGameObject->getSpatial()->mW;
    }

    const glm::mat4 & CameraComponent::getView() const {
        if (mViewMatDirty) {
            _detView();
        }
        return mViewMat;
    }

    const glm::mat4 & CameraComponent::getProj() const {
        if (mProjMatDirty) {
            _detProj();
        }
        return mProjMat;
    }

    void CameraComponent::_detView() const {
        auto spatial = mGameObject->getSpatial();
        mViewMat = glm::lookAt(spatial->getPosition(), spatial->getPosition() + getLookDir(), spatial->mV);
        mViewMatDirty = false;
    }

    void CameraComponent::_detProj() const {
        if (mIsOrtho) {
            mProjMat = glm::ortho(mHorizBounds.x, mHorizBounds.y, mVertBounds.x, mVertBounds.y, mNear, mFar);
        }
        else {
            mProjMat = glm::perspective(glm::radians(mFOV), Window::getAspectRatio(), mNear, mFar);
        }
        mProjMatDirty = false;
    }

}