#include "OrthoCameraComponent.hpp"
#include "Component/SpatialComponent/SpatialComponent.hpp"

#include "GameObject/GameObject.hpp"
#include "Messaging/Messenger.hpp"

#include "Window/Window.hpp"

#include "glm/gtc/matrix_transform.hpp"

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
        mProjMat = glm::ortho(mHorizBounds.x, mHorizBounds.y, mVertBounds.x, mVertBounds.y, mNear, mFar);
        mProjMatDirty = false;
    }

}