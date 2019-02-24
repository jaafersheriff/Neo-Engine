/* Free camera */
#pragma once

#include "Component/Component.hpp"

namespace neo {

    class CameraComponent;

    class CameraControllerComponent : public Component {

        public: 
            float mLookSpeed;
            float mMoveSpeed;

            CameraControllerComponent(GameObject *, float ls, float ms);
            CameraControllerComponent(CameraControllerComponent && other) = default;

            virtual void update(float) override;

            void setOrientation(float p, float t);
            void setButtons(int, int, int, int, int, int);
            float getTheta() const { return mTheta; }
            float getPhi() const { return mPhi; }

        private:
            void _updateSpatialOrientation();

            float mTheta, mPhi;

            int mForwardButton;
            int mBackwardButton;
            int mRightButton;
            int mLeftButton;
            int mUpButton;
            int mDownButton;
    };
}