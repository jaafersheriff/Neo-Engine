#include "CameraComponent.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"

// #include "ECS/Messaging/Messenger.hpp"

#include "Util/Util.hpp"

#include "glm/gtc/matrix_transform.hpp"
#include <imgui/imgui.h>
#include "microprofile.h"

namespace neo {

    CameraComponent::CameraComponent()
        : mNear()
        , mFar()
        , mProjMat()
        , mProjMatDirty(true)
    {
    }

    void CameraComponent::setNearFar(float near, float far) {
        if (near == mNear && far == mFar) {
            return;
        }

        mNear = near;
        mFar = far;
        mProjMatDirty = true;
    }

    const glm::mat4 & CameraComponent::getProj() const {
        if (mProjMatDirty) {
            _detProj();
        }
        return mProjMat;
    }

    void CameraComponent::imGuiEditor() {
        if (ImGui::SliderFloat("Near", &mNear, 0.1f, 10.f)) {
            mProjMatDirty = true;
        }
        if (ImGui::SliderFloat("Far", &mFar, 1.f, 100.f)) {
            mProjMatDirty = true;
        }
    }

}
