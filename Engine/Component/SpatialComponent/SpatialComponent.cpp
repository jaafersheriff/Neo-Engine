#include "SpatialComponent.hpp"

#include "glm/gtc/matrix_transform.hpp"

namespace neo {

    SpatialComponent::SpatialComponent(GameObject & go) :
        Component(go),
        Orientable(),
        position(),
        scale(1.f),
        modelMatrix(),
        normalMatrix(),
        modelMatrixDirty(true),
        normalMatrixDirty(true)
    {}

    SpatialComponent::SpatialComponent(GameObject & go, const glm::vec3 & p, const glm::vec3 & s) :
        Component(go),
        Orientable(),
        position(p),
        scale(s),
        modelMatrix(),
        normalMatrix(),
        modelMatrixDirty(true),
        normalMatrixDirty(true)
    {}

    SpatialComponent::SpatialComponent(GameObject &go, const glm::vec3 & p, const glm::vec3 & s, const glm::mat3 & o) :
        SpatialComponent(go, p, s) {
        setOrientation(o);
    }

    void SpatialComponent::move(const glm::vec3 & delta) {
        position += delta;
        modelMatrixDirty = true;
    }

    void SpatialComponent::resize(const glm::vec3 & factor) {
        scale *= factor;
        modelMatrixDirty = true;
        normalMatrixDirty = true;
    }

    void SpatialComponent::rotate(const glm::mat3 & mat) {
        Orientable::rotate(mat);
        modelMatrixDirty = true;
        normalMatrixDirty = true;
    }

    void SpatialComponent::setPosition(const glm::vec3 & loc) {
        position = loc;
        modelMatrixDirty = true;
    }

    void SpatialComponent::setScale(const glm::vec3 & scale) {
        this->scale = scale;
        modelMatrixDirty = true;
        normalMatrixDirty = true;
    }

    void SpatialComponent::setOrientation(const glm::mat3 & orient) {
        Orientable::setOrientation(orient);
        modelMatrixDirty = true;
        normalMatrixDirty = true;
    }

    void SpatialComponent::setUVW(const glm::vec3 & u, const glm::vec3 & v, const glm::vec3 & w) {
        Orientable::setUVW(u, v, w);
        modelMatrixDirty = true;
        normalMatrixDirty = true;
    }
        
    const glm::mat4 & SpatialComponent::getModelMatrix() const {
        if (modelMatrixDirty) detModelMatrix();
        return modelMatrix;
    }

    const glm::mat3 & SpatialComponent::getNormalMatrix() const {
        if (normalMatrixDirty) detNormalMatrix();
        return normalMatrix;
    }

    void SpatialComponent::detModelMatrix() const {
        modelMatrix = glm::scale(glm::translate(glm::mat4(), position) * glm::mat4(getOrientation()), scale);
        modelMatrixDirty = false;
    }

    void SpatialComponent::detNormalMatrix() const {
        normalMatrix = getOrientation() * glm::mat3(glm::scale(glm::mat4(), 1.0f / scale));
        normalMatrixDirty = false;
    }
}
