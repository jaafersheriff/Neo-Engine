#include "PerspectiveCameraComponent.hpp"
#include "Component/SpatialComponent/SpatialComponent.hpp"

#include "GameObject/GameObject.hpp"
#include "Messaging/Messenger.hpp"

#include "glm/gtc/matrix_transform.hpp"

namespace neo {

    PerspectiveCameraComponent::PerspectiveCameraComponent(GameObject *gameObject, float near, float far, float fov, float ar) :
        CameraComponent(gameObject) {
        setNearFar(near, far);
        setFOV(fov);
        setAspectRatio(ar);
    }

    void PerspectiveCameraComponent::setFOV(float fov) {
        if (fov == mFOV) {
            return;
        }

        mFOV = fov;
        mProjMatDirty = true;
    }

    void PerspectiveCameraComponent::setAspectRatio(float ar) {
        if (ar == mAspectRatio) {
            return;
        }

        mAspectRatio = ar;
        mProjMatDirty = true;
    }


    void PerspectiveCameraComponent::_detProj() const {
        mProjMat = glm::perspective(glm::radians(mFOV), mAspectRatio, mNear, mFar);
        mProjMatDirty = false;
    }

}