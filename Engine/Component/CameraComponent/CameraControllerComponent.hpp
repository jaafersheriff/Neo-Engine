/* Free camera */
#pragma once

#include "Component/Component.hpp"

namespace neo {

    class CameraComponent;

    class CameraControllerComponent : public Component {

        public: 
            CameraControllerComponent(GameObject &, float ls, float ms);
            CameraControllerComponent(CameraControllerComponent && other) = default;

            virtual void init() override;
            virtual void update(float) override;

            void setOrientation(float p, float t);
            void setButtons(int, int, int, int, int, int);
            float getTheta() const { return theta; }
            float getPhi() const { return phi; }

        private:
            void updateSpatialOrientation();

            SpatialComponent *spatial;
            float theta, phi;
            float lookSpeed;
            float minMoveSpeed, maxMoveSpeed;
            float moveSpeed;

            int forwardButton;
            int backwardButton;
            int rightButton;
            int leftButton;
            int upButton;
            int downButton;
    };
}