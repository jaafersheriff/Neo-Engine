#include "ECS/pch.hpp"
#include "PerspectiveCameraComponent.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"

namespace neo {

    PerspectiveCameraComponent::PerspectiveCameraComponent(float near, float far, float fov, float ar) :
        CameraComponent() {
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
        ZoneScoped;
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
