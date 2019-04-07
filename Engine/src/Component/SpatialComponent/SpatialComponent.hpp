#pragma once

#include "Component/Component.hpp"
#include "Orientable.hpp"

#include <glm/glm.hpp>

namespace neo {

    class SpatialComponent : public Component, public Orientable {

        public:
            glm::vec3 mPosition;
            glm::vec3 mScale;

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
            const glm::mat4 & getModelMatrix() const;
            const glm::mat3 & getNormalMatrix() const;

        private:
            void _detModelMatrix() const;
            void _detNormalMatrix() const;
            mutable glm::mat4 mModelMatrix;
            mutable glm::mat3 mNormalMatrix;
            mutable bool mModelMatrixDirty;
            mutable bool mNormalMatrixDirty;
    };

};

