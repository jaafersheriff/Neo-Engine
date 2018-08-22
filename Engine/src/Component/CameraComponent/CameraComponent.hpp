#pragma once

#include "Component/Component.hpp"

#include <glm/glm.hpp>

namespace neo {

    class SpatialComponent;

    class CameraComponent : public Component {

        public:
            CameraComponent(GameObject *, float fov, float near, float far);
            CameraComponent(GameObject *, float hMin, float hMax, float vMin, float vMax, float near, float far);
            CameraComponent(CameraComponent &&) = default;

            virtual void init() override;

            /* Setters */
            void setFOV(float);
            void setNearFar(float, float);
            void setOrthoBounds(const glm::vec2 &, const glm::vec2 &);
            void setLookAt(SpatialComponent *);
            void setLookDir(glm::vec3);

            /* Getters */
            const float getFOV() const { return fov; }
            const float getNear() const { return near; }
            const float getFar() const { return far; }
            const glm::vec2 getHorizontalBounds() const { return horizBounds; }
            const glm::vec2 getVerticalBounds() const { return vertBounds; }
            const glm::vec3 getLookDir() const;
            const glm::vec3 getLookPos() const;
            const glm::mat4 & getView() const;
            const glm::mat4 & getProj() const;

        private:
            float fov;
            float near, far;
            glm::vec2 horizBounds;
            glm::vec2 vertBounds;
            bool isOrtho;

            SpatialComponent *lookAt;

            void detView() const;
            void detProj() const;

            /* Should never be used directly -- call getters */
            mutable glm::mat4 viewMat;
            mutable glm::mat4 projMat;
            mutable bool viewMatDirty;
            mutable bool projMatDirty;

    };

}