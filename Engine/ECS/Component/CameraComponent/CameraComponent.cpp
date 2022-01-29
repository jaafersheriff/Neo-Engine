#include "CameraComponent.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"

#include "ECS/GameObject.hpp"
#include "Messaging/Messenger.hpp"

#include "Util/Util.hpp"

#include "glm/gtc/matrix_transform.hpp"
#include "ext/imgui/imgui.h"

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
            NEO_UNUSED(msg_);
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
        MICROPROFILE_SCOPEI("CameraComponent", "_detView", MP_AUTO);
        auto spatial = mGameObject->getComponentByType<SpatialComponent>();
        NEO_ASSERT(spatial, "Camera has no SpatialComponent");
        mViewMat = glm::lookAt(spatial->getPosition(), spatial->getPosition() + spatial->getLookDir(), spatial->getUpDir());
        mViewMatDirty = false;
    }

    void CameraComponent::imGuiEditor() {
        ImGui::SliderFloat("Near", &mNear, 0.1f, 10.f);
        ImGui::SliderFloat("Far", &mFar, 1.f, 100.f);
    }

}