#pragma once

#include "Component/Component.hpp"

#include <glm/glm.hpp>

namespace neo {

    class CameraComponent : public Component {

        public:
            CameraComponent(GameObject &, float fov, float near, float far);
            CameraComponent(CameraComponent &&) = default;

            virtual void init() override;

            /* Setters */
            void setFOV(float);
            void setNearFar(float, float);

            /* Getters */
            const float getFOV() const { return fov; }
            const float getNear() const { return near; }
            const float getFar() const { return far; }
            const glm::vec3 getLookDir() const;
            const glm::mat4 & getView() const;
            const glm::mat4 & getProj() const;

        private:
            float fov;
            float near, far;

            void detView() const;
            void detProj() const;

            /* Should never be used directly -- call getters */
            mutable glm::mat4 viewMat;
            mutable glm::mat4 projMat;
            mutable bool viewMatDirty;
            mutable bool projMatDirty;

    };

}