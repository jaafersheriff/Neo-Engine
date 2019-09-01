#include "PerspectiveCameraComponent.hpp"
#include "Component/SpatialComponent/SpatialComponent.hpp"

#include "GameObject/GameObject.hpp"
#include "Messaging/Messenger.hpp"

#include "Window/Window.hpp"

#include "glm/gtc/matrix_transform.hpp"

namespace neo {

    PerspectiveCameraComponent::PerspectiveCameraComponent(GameObject *gameObject, float near, float far, float fov) :
        CameraComponent(gameObject) {
        setNearFar(near, far);
        setFOV(fov);
    }

    void PerspectiveCameraComponent::setFOV(float fov) {
        if (fov == mFOV) {
            return;
        }

        mFOV = fov;
        mProjMatDirty = true;
    }

    void PerspectiveCameraComponent::_detProj() const {
        // TODO - aspect ratio shouldn't ALWAYS come from window
        mProjMat = glm::perspective(glm::radians(mFOV), Window::getAspectRatio(), mNear, mFar);
        mProjMatDirty = false;
    }

}