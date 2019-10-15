#include "PerspectiveCameraComponent.hpp"
#include "Component/SpatialComponent/SpatialComponent.hpp"

#include "GameObject/GameObject.hpp"
#include "Messaging/Messenger.hpp"

#include "glm/gtc/matrix_transform.hpp"

#include "ext/imgui/imgui.h"

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

    void PerspectiveCameraComponent::imGuiEditor() {
        CameraComponent::imGuiEditor();
        float fov = getFOV();
        float ar = getAspectRatio();
        if (ImGui::SliderFloat("FOV", &fov, 0.f, 180.f)) {
            setFOV(fov);
        }
        if (ImGui::SliderFloat("Aspect Ratio", &ar, 0.f, 1.f)) {
            setAspectRatio(ar);
        }
    }

}