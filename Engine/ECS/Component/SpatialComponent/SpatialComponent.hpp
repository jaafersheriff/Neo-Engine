#pragma once

#include "ECS/Component/Component.hpp"
#include "Orientable.hpp"

#include <glm/glm.hpp>

namespace neo {

    class SpatialComponent : public Component, public Orientable {

        public:

            SpatialComponent(GameObject *);
            SpatialComponent(GameObject *, const glm::vec3 &);
            SpatialComponent(GameObject *, const glm::vec3 &, const glm::vec3 &);
            SpatialComponent(GameObject *, const glm::vec3 &, const glm::vec3 &, const glm::vec3 &);
            SpatialComponent(GameObject *, const glm::vec3 &, const glm::vec3 &, const glm::mat3 &);

            SpatialComponent(SpatialComponent && other) = default;

            /* Update */
            void move(const glm::vec3 &);
            void resize(const glm::vec3 &);
            void rotate(const glm::mat3 &);
            virtual void imGuiEditor() override;

            /* Setters */
            void setPosition(const glm::vec3 &);
            void setScale(const glm::vec3 &);
            void setScale(const float);
            void setOrientation(const glm::mat3 &);
            void setUVW(const glm::vec3 &, const glm::vec3 &, const glm::vec3 &);
            void setDirty();

            /* Getters */
            const glm::vec3 getPosition() const { return mPosition; }
            const glm::vec3 getScale() const { return mScale; }
            const glm::mat4 & getModelMatrix() const;
            const glm::mat3 & getNormalMatrix() const;

        private:
            glm::vec3 mPosition{ 0.f, 0.f, 0.f };
            glm::vec3 mScale{ 1.f, 1.f, 1.f };

            void _detModelMatrix() const;
            void _detNormalMatrix() const;
            mutable glm::mat4 mModelMatrix;
            mutable glm::mat3 mNormalMatrix;
            mutable bool mModelMatrixDirty;
            mutable bool mNormalMatrixDirty;
    };

};

