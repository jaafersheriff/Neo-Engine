#include "CameraComponent.hpp"
#include "Component/SpatialComponent/SpatialComponent.hpp"

#include "GameObject/GameObject.hpp"
#include "Messaging/Messenger.hpp"

#include "Window/Window.hpp"

#include "glm/gtc/matrix_transform.hpp"

namespace neo {

    CameraComponent::CameraComponent(GameObject *gameObject, float near, float far) :
        Component(gameObject),
        near(near),
        far(far),
        fov(0.f),
        horizBounds(),
        vertBounds(),
        viewMat(),
        projMat(),
        viewMatDirty(true),
        projMatDirty(true)
    {}

    CameraComponent::CameraComponent(GameObject *gameObject, float fov, float near, float far) :
        CameraComponent(gameObject, near, far) {
        setFOV(fov);
        isOrtho = false;
    }

    CameraComponent::CameraComponent(GameObject *gameObject, float horizMin, float horizMax, float vertMin, float vertMax, float near, float far) :
        CameraComponent(gameObject, near, far) {
        setOrthoBounds(glm::vec2(horizMin, horizMax), glm::vec2(vertMin, vertMax));
        isOrtho = true;
    }

    void CameraComponent::init() {
        viewMatDirty = true;
        projMatDirty = true;

        Messenger::addReceiver<SpatialChangeMessage>(gameObject, [&](const Message & msg_) {
            viewMatDirty = true;
        });

        if (!isOrtho) {
            Messenger::addReceiver<WindowFrameSizeMessage>(nullptr, [&] (const Message & msg_) {
                projMatDirty = true;
            });
        }
    }

    void CameraComponent::setFOV(float fov) {
        if (fov == this->fov) {
            return;
        }

        this->fov = fov;
        projMatDirty = true;
    }

    void CameraComponent::setNearFar(float near, float far) {
        if (near == this->near && far == this->far) {
            return;
        }

        this->near = near;
        this->far = far;
        projMatDirty = true;
    }

    void CameraComponent::setOrthoBounds(const glm::vec2 &h, const glm::vec2 &v) {
        if (h == this->horizBounds && v == this->vertBounds) {
            return;
        }

        this->horizBounds = h;
        this->vertBounds = v;
        projMatDirty = true;
        viewMatDirty = true;
    }

    void CameraComponent::setLookDir(glm::vec3 dir) {
        glm::vec3 w = -glm::normalize(dir);
        auto spatial = gameObject->getSpatial();
        if (w == spatial->getW()) {
            return;
        }
        glm::vec3 u = glm::cross(w, glm::vec3(0, 1, 0));
        glm::vec3 v = glm::cross(u, w);
        u = glm::cross(v, w);
        spatial->setUVW(u, v, w);
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
            projMat = glm::perspective(glm::radians(fov), Window::getAspectRatio(), near, far);
        }
        projMatDirty = false;
    }

}