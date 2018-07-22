#include "CameraComponent.hpp"

#include "GameObject/GameObject.hpp"
#include "Component/SpatialComponent/SpatialComponent.hpp"

#include "Window/Window.hpp"

#include "glm/gtc/matrix_transform.hpp"

namespace neo {

    CameraComponent::CameraComponent(GameObject &gameObject, float fov, float near, float far) :
         Component(gameObject),
         fov(fov),
         near(near),
         far(far),
         viewMat(),
         projMat(),
         viewMatDirty(true),
         projMatDirty(true)
    {}

    void CameraComponent::init() {
        viewMatDirty = true;
        projMatDirty = true;
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
        glm::vec3 t(-spatial->getPosition());
        const glm::vec3 & u = spatial->getU();
        const glm::vec3 & v = spatial->getV();
        const glm::vec3 & w = spatial->getW();
        viewMat = glm::mat4(
                       u.x,            v.x,            w.x, 0.f,
                       u.y,            v.y,            w.y, 0.f,
                       u.z,            v.z,            w.z, 0.f,
            glm::dot(u, t), glm::dot(v, t), glm::dot(w, t), 1.f
        );
        viewMatDirty = false;
    }

    void CameraComponent::detProj() const {
        projMat = glm::perspective(fov, Window::getAspectRatio(), near, far);
        projMatDirty = false;
    }
}