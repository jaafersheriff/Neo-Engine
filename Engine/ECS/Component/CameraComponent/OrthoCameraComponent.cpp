#include "OrthoCameraComponent.hpp"

#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"
#include "ECS/Messaging/Messenger.hpp"
#include "Util/Util.hpp"

#include "glm/gtc/matrix_transform.hpp"
#include "ext/imgui/imgui.h"

namespace neo {

    OrthoCameraComponent::OrthoCameraComponent(GameObject *gameObject, float near, float far, float horizMin, float horizMax, float vertMin, float vertMax) 
        : CameraComponent(gameObject) {
        setNearFar(near, far);
        setOrthoBounds(glm::vec2(horizMin, horizMax), glm::vec2(vertMin, vertMax));
    }

    void OrthoCameraComponent::setOrthoBounds(const glm::vec2 &h, const glm::vec2 &v) {
        if (h == mHorizBounds && v == mVertBounds) {
            return;
        }

        mHorizBounds = h;
        mVertBounds = v;
        mProjMatDirty = true;
        mViewMatDirty = true;
    }

    void OrthoCameraComponent::_detProj() const {
        MICROPROFILE_SCOPEI("OrthoCameraComponent", "OrthoCameraComponent::_detProj", MP_AUTO);
        mProjMat = glm::ortho(mHorizBounds.x, mHorizBounds.y, mVertBounds.x, mVertBounds.y, mNear, mFar);
        mProjMatDirty = false;
    }

    void OrthoCameraComponent::imGuiEditor() {
        CameraComponent::imGuiEditor();
        glm::vec2 h = getHorizontalBounds();
        glm::vec2 v = getVerticalBounds();
        bool edited = false;
        edited = edited || ImGui::SliderFloat2("H-Bounds", &h[0], -10.f, 10.f);
        edited = edited || ImGui::SliderFloat2("V-Bounds", &v[0], -10.f, 10.f);
        if (edited) {
            setOrthoBounds(h, v);
        }
    }

}
