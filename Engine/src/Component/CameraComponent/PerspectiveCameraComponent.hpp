#pragma once

#include "Component/Component.hpp"
#include "Component/CameraComponent/CameraComponent.hpp"

#include <glm/glm.hpp>

namespace neo {

    class PerspectiveCameraComponent : public CameraComponent {

        public:
            PerspectiveCameraComponent(GameObject *, float near, float far, float fov);
            PerspectiveCameraComponent(PerspectiveCameraComponent &&) = default;

            /* Setters */
            void setFOV(float);

            /* Getters */
            const float getFOV() const { return mFOV; }

        private:
            float mFOV;

            virtual void _detProj() const override;

    };

}