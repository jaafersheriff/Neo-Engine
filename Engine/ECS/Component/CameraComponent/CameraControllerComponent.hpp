/* Free camera */
#pragma once

#include "ECS/Component/Component.hpp"

namespace neo {

    struct CameraControllerComponent : public Component {
        float mLookSpeed;
        float mMoveSpeed;

        int mLookLeftButton;
        int mLookRightButton;
        int mLookDownButton;
        int mLookUpButton;

        int mForwardButton;
        int mBackwardButton;
        int mRightButton;
        int mLeftButton;
        int mUpButton;
        int mDownButton;

        CameraControllerComponent(float ls, float ms);
        virtual std::string getName() { return "CameraControllerComponent"; };
        virtual void imGuiEditor();

    private:
        void _updateSpatialOrientation();

    };
}