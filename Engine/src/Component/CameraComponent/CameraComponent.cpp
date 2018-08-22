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
        projMatDirty(true),
        lookAt(nullptr)
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
        projMatDirty(true),
        lookAt(nullptr)
    {}

    void CameraComponent::init() {
        viewMatDirty = true;
        projMatDirty = true;

        Messenger::addReceiver<SpatialChangeMessage>(gameObject, [&](const Message & msg_) {
            viewMatDirty = true;
        });
    }

    void CameraComponent::setLookAt(SpatialComponent *spat) {
        if (lookAt) {
            // remove receiver functions
        }

        lookAt = spat;

        auto lookAtCallback([&](const Message & msg_) {
            glm::vec3 lookPos = lookAt->getPosition();
            glm::vec3 thisPos = this->getGameObject().getSpatial()->getPosition();
            glm::vec3 diff = lookPos - thisPos;
            setLookDir(diff);
        });

        Messenger::addReceiver<SpatialChangeMessage>(&spat->getGameObject(), lookAtCallback);
        Messenger::addReceiver<SpatialChangeMessage>(gameObject, lookAtCallback);
    }

    const glm::vec3 CameraComponent::getLookPos() const {
        return lookAt ? lookAt->getPosition() : glm::vec3(0.f);
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

    void CameraComponent::setLookDir(glm::vec3 dir) {
        auto spatial = gameObject->getSpatial();
        glm::vec3 w = -glm::normalize(dir);
        glm::vec3 u = glm::cross(w, glm::vec3(0,1,0));
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
            projMat = glm::perspective(fov, Window::getAspectRatio(), near, far);
        }
        projMatDirty = false;
    }
}