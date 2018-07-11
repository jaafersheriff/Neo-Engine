#include "CameraComponent.hpp"
#include "GameObject/GameObject.hpp"

#include "Window/Window.hpp"

#include "glm/gtc/matrix_transform.hpp"

namespace neo {

    CameraComponent::CameraComponent(GameObject &gameObject, float fov, float, float, glm::vec3 pos, glm::vec3 lookAt) : 
         Component(gameObject),
         position(position),
         lookAt(lookAt),
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
        return glm::normalize(lookAt - position);
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
        viewMat = glm::lookAt(position, lookAt, glm::vec3(0, 1, 0));
        viewMatDirty = false;
    }

    void CameraComponent::detProj() const {
        projMat = glm::perspective(fov, Window::getAspectRatio(), near, far);
        projMatDirty = false;
    }
}