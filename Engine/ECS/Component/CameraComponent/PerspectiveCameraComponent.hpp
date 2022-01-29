#pragma once

#include "ECS/Component/Component.hpp"
#include "ECS/Component/CameraComponent/CameraComponent.hpp"

#include <glm/glm.hpp>

namespace neo {

    class PerspectiveCameraComponent : public CameraComponent {

        public:
            PerspectiveCameraComponent(GameObject *, float near, float far, float fov, float ar);
            PerspectiveCameraComponent(PerspectiveCameraComponent &&) = default;

            virtual void imGuiEditor() override;

            /* Setters */
            void setFOV(float);
            void setAspectRatio(float);

            /* Getters */
            const float getFOV() const { return mFOV; }
            const float getAspectRatio() const { return mAspectRatio; }

        private:
            float mFOV;
            float mAspectRatio;

            virtual void _detProj() const override;

    };

}