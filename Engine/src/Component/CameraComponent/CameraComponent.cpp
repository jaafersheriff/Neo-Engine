#include "CameraComponent.hpp"
#include "Component/SpatialComponent/SpatialComponent.hpp"

#include "GameObject/GameObject.hpp"
#include "Messaging/Messenger.hpp"

#include "Window/Window.hpp"

#include "glm/gtc/matrix_transform.hpp"

namespace neo {

    CameraComponent::CameraComponent(GameObject *gameObject) :
        Component(gameObject),
        mNear(),
        mFar(),
        mViewMat(),
        mProjMat(),
        mViewMatDirty(true),
        mProjMatDirty(true)
    {}

    void CameraComponent::init() {
        mViewMatDirty = true;
        mProjMatDirty = true;

        Messenger::addReceiver<SpatialChangeMessage>(mGameObject, [&](const Message & msg_) {
            mViewMatDirty = true;
        });
    }

    void CameraComponent::setNearFar(float near, float far) {
        if (near == mNear && far == mFar) {
            return;
        }

        mNear = near;
        mFar = far;
        mProjMatDirty = true;
    }

    void CameraComponent::setLookDir(glm::vec3 dir) {
        glm::vec3 w = -glm::normalize(dir);
        auto spatial = mGameObject->getSpatial();
        if (w == -spatial->getLookDir()) {
            return;
        }
        glm::vec3 u = glm::cross(w, glm::vec3(0, 1, 0));
        glm::vec3 v = glm::cross(u, w);
        u = glm::cross(v, w);
        spatial->setUVW(u, v, w);
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
        mViewMat = glm::lookAt(spatial->getPosition(), spatial->getPosition() + spatial->getLookDir(), spatial->getUpDir());
        mViewMatDirty = false;
    }

}