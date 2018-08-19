#include "CameraComponent.hpp"
#include "Component/SpatialComponent/SpatialComponent.hpp"

#include "GameObject/GameObject.hpp"
#include "Messaging/Messenger.hpp"

#include "Window/Window.hpp"

#include "glm/gtc/matrix_transform.hpp"

namespace neo {

    CameraComponent::CameraComponent(GameObject *gameObject, float fov, float near, float far) :
        Component(gameObject),
        fov(fov),
        near(near),
        far(far),
        isOrtho(false),
        horizBounds(),
        vertBounds(),
        viewMat(),
        projMat(),
        viewMatDirty(true),
        projMatDirty(true)
    {}

    CameraComponent::CameraComponent(GameObject *gameObject, float horizMin, float horizMax, float vertMin, float vertMax, float near, float far) :
        Component(gameObject),
        fov(0.f),
        near(near),
        far(far),
        isOrtho(true),
        horizBounds(glm::vec2(horizMin, horizMax)),
        vertBounds(glm::vec2(vertMin, vertMax)),
        viewMat(),
        projMat(),
        viewMatDirty(true),
        projMatDirty(true)
    {}

    void CameraComponent::init() {
        viewMatDirty = true;
        projMatDirty = true;

        Messenger::addReceiver<SpatialChangeMessage>(gameObject, [&](const Message & msg_) {
            viewMatDirty = true;
        });
    }

    void CameraComponent::setFOV(float fov) {
        this->fov = fov;
        projMatDirty = true;
    }

    void CameraComponent::setNearFar(float near, float far) {
        this->near = near;
        this->far = far;
        projMatDirty = true;
    }

    void CameraComponent::setOrthoBounds(const glm::vec2 &h, const glm::vec2 &v) {
        this->horizBounds = h;
        this->vertBounds = v;
        projMatDirty = true;
        viewMatDirty = true;
    }

    const glm::vec3 CameraComponent::getLookDir() const {
        return -gameObject->getSpatial()->getW();
    }

    const glm::mat4 & CameraComponent::getView() const {
        if (viewMatDirty) {
            detView();
        }
        return viewMat;
    }

    const glm::mat4 & CameraComponent::getProj() const {
        if (projMatDirty) {
            detProj();
        }
        return projMat;
    }

    void CameraComponent::detView() const {
        auto spatial = gameObject->getSpatial();
        viewMat = glm::lookAt(spatial->getPosition(), spatial->getPosition() + getLookDir(), spatial->getV());
        viewMatDirty = false;
    }

    void CameraComponent::detProj() const {
        if (isOrtho) {
            projMat = glm::ortho(horizBounds.x, horizBounds.y, vertBounds.x, vertBounds.y, near, far);
        }
        else {
            projMat = glm::perspective(fov, Window::getAspectRatio(), near, far);
        }
        projMatDirty = false;
    }
}