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

		float mTheta;
		float mPhi;

		CameraControllerComponent(float ls, float ms);
		virtual std::string getName() const override { return "CameraControllerComponent"; };
		virtual void imGuiEditor();

	};
}