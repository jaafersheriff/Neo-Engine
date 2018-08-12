/* Free camera */
#pragma once

#include "Component/Component.hpp"

namespace neo {

    class CameraComponent;

    class CameraControllerComponent : public Component {

        public: 
            CameraControllerComponent(GameObject *, float ls, float ms);
            CameraControllerComponent(CameraControllerComponent && other) = default;

            virtual void update(float) override;

            float lookSpeed;
            float moveSpeed;

            void setOrientation(float p, float t);
            void setButtons(int, int, int, int, int, int);
            float getTheta() const { return theta; }
            float getPhi() const { return phi; }

        private:
            void updateSpatialOrientation();

            float theta, phi;

            int forwardButton;
            int backwardButton;
            int rightButton;
            int leftButton;
            int upButton;
            int downButton;
    };
}