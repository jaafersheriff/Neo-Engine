/* Free camera */
#pragma once

#include "ECS/Component/Component.hpp"

namespace neo {

	START_COMPONENT(CameraControllerComponent);
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

		float mTheta;
		float mPhi;

		CameraControllerComponent(float ls, float ms);
		virtual void imGuiEditor();
	END_COMPONENT();
}