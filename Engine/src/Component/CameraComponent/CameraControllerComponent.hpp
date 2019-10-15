/* Free camera */
#pragma once

#include "Component/Component.hpp"

namespace neo {

    class CameraComponent;

    class CameraControllerComponent : public Component {

        public: 
            float mLookSpeed;
            float mMoveSpeed;
            float mTheta, mPhi;

            int mForwardButton;
            int mBackwardButton;
            int mRightButton;
            int mLeftButton;
            int mUpButton;
            int mDownButton;
 
            CameraControllerComponent(GameObject *, float ls, float ms);
            CameraControllerComponent(CameraControllerComponent && other) = default;
            virtual void imGuiEditor();

            void setOrientation(float p, float t);

        private:
            void _updateSpatialOrientation();

   };
}