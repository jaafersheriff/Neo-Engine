#pragma once

#include "Component/Component.hpp"
#include "Orientable.hpp"

#include <glm/glm.hpp>

namespace neo {

    class SpatialComponent : public Component, public Orientable {

        public:

            SpatialComponent(GameObject *);
            SpatialComponent(GameObject *, const glm::vec3 &);
            SpatialComponent(GameObject *, const glm::vec3 &, const glm::vec3 &);
            SpatialComponent(GameObject *, const glm::vec3 &, const glm::vec3 &, const glm::mat3 &);

            SpatialComponent(SpatialComponent && other) = default;

            /* Update */
            void move(const glm::vec3 &);
            void resize(const glm::vec3 &);
            void rotate(const glm::mat3 &);

            /* Setters */
            void setPosition(const glm::vec3 &);
            void setScale(const glm::vec3 &);
            void setOrientation(const glm::mat3 &);
            void setUVW(const glm::vec3 &, const glm::vec3 &, const glm::vec3 &);

            /* Getters */
            const glm::vec3 & getPosition() const { return position; }
            const glm::vec3 & getScale() const { return scale; }
            const glm::mat4 & getModelMatrix() const;
            const glm::mat3 & getNormalMatrix() const;

        private:
            glm::vec3 position;
            glm::vec3 scale;

            void detModelMatrix() const;
            void detNormalMatrix() const;
            mutable glm::mat4 modelMatrix;
            mutable glm::mat3 normalMatrix;
            mutable bool modelMatrixDirty;
            mutable bool normalMatrixDirty;
    };

};

