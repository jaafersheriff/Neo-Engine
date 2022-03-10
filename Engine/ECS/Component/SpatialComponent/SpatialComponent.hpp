#pragma once

#include "ECS/ECS.hpp"
#include "ECS/Component/Component.hpp"
#include "Orientable.hpp"

#include <glm/glm.hpp>

namespace neo {

    struct SpatialComponent : public Component, public Orientable {

            SpatialComponent();
            SpatialComponent(const glm::vec3 &);
            SpatialComponent(const glm::vec3 &, const glm::vec3 &);
            SpatialComponent(const glm::vec3 &, const glm::vec3 &, const glm::vec3 &);
            SpatialComponent(const glm::vec3 &, const glm::vec3 &, const glm::mat3 &);

            virtual std::string getName() const override { return "SpatialComponent"; }
            virtual void imGuiEditor() override;

            /* Update */
            void move(const glm::vec3 &);
            void resize(const glm::vec3 &);
            void rotate(const glm::mat3 &);

            /* Setters */
            void setPosition(const glm::vec3 &);
            void setScale(const glm::vec3 &);
            void setScale(const float);
            void setOrientation(const glm::mat3 &);
            void setUVW(const glm::vec3 &, const glm::vec3 &, const glm::vec3 &);
            void setDirty();

            /* Getters */
            glm::vec3 getPosition() const { return mPosition; }
            glm::vec3 getScale() const { return mScale; }
            const glm::mat4& getModelMatrix() const;
            const glm::mat3& getNormalMatrix() const;
            const glm::mat4& getView() const;

        private:
            glm::vec3 mPosition{ 0.f, 0.f, 0.f };
            glm::vec3 mScale{ 1.f, 1.f, 1.f };

            void _detModelMatrix() const;
            void _detNormalMatrix() const;
            void _detView() const;
            mutable glm::mat4 mModelMatrix;
            mutable glm::mat3 mNormalMatrix;
            mutable glm::mat4 mViewMat;
            mutable bool mModelMatrixDirty;
            mutable bool mNormalMatrixDirty;
            mutable bool mViewMatDirty;
    };

};

