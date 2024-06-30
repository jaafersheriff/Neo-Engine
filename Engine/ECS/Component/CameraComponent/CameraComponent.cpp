#include "ECS/pch.hpp"
#include "CameraComponent.hpp"

namespace neo {

	CameraComponent::CameraComponent(float near, float far, Perspective inCamera) 
		: CameraComponent(near, far, CameraType::Perspective) {
		mPerspective = inCamera;
	}

	CameraComponent::CameraComponent(float near, float far, Orthographic inCamera)
		: CameraComponent(near, far, CameraType::Orthographic) {
		mOrthographic = inCamera;
	}

	CameraComponent::CameraComponent(float near, float far, CameraType type) 
		: mNear(near)
		, mFar(far)
		, mType(type)
		, mProjMatDirty(true)
	{}

	void CameraComponent::setNear(float near) {
		if (near != mNear) {
			mNear = near;
			mProjMatDirty = true;
		}
	}

	void CameraComponent::setFar(float far) {
		if (far != mFar) {
			mFar = far;
			mProjMatDirty = true;
		}
	}

	void CameraComponent::setPerspective(Perspective camera) {
		NEO_ASSERT(mPerspective.has_value(), "This isn't an perspective camera..");
		if (camera == *mPerspective) {
			return;
		}
		mPerspective = camera;
		mProjMatDirty = true;
	}

	void CameraComponent::setOrthographic(Orthographic camera) {
		NEO_ASSERT(mOrthographic.has_value(), "This isn't an orthograhpic camera..");
		if (camera == *mOrthographic) {
			return;
		}
		mOrthographic = camera;
		mProjMatDirty = true;
	}

	const glm::mat4 & CameraComponent::getProj() const {
		if (mProjMatDirty) {
			if (mPerspective) {
				mProjMat = glm::perspective(
					glm::radians(mPerspective->mFOV), 
					mPerspective->mAspectRatio, 
					mNear, 
					mFar);
			}
			else if (mOrthographic) {
				mProjMat = glm::ortho(
					mOrthographic->mHorizBounds.x, 
					mOrthographic->mHorizBounds.y, 
					mOrthographic->mVertBounds.x, 
					mOrthographic->mVertBounds.y, 
					mNear, 
					mFar);
			}
			else {
				NEO_FAIL("This should never happen");
			}
			mProjMatDirty = false;
		}
		return mProjMat;
	}

	CameraComponent::CameraType CameraComponent::getType() const {
		return mType;
	}

	float CameraComponent::getNear() const {
		return mNear;
	}

	float CameraComponent::getFar() const {
		return mFar;
	}

	const CameraComponent::Perspective& CameraComponent::getPerspective() const {
		NEO_ASSERT(mPerspective.has_value(), "This isn't a perspective camera..");

		return *mPerspective;
	}

	const CameraComponent::Orthographic& CameraComponent::getOrthographic() const {
		NEO_ASSERT(mOrthographic.has_value(), "This isn't an orthograhpic camera..");

		return *mOrthographic;
	}

	void CameraComponent::imGuiEditor() {
		mProjMatDirty |= ImGui::SliderFloat("Near", &mNear, 0.1f, 10.f);
		mProjMatDirty |= ImGui::SliderFloat("Far", &mFar, 1.f, 100.f);

		if (mPerspective) {
			mProjMatDirty |= mPerspective->imGuiEditor();
		}
		if (mOrthographic) {
			mProjMatDirty |= mOrthographic->imGuiEditor();
		}
	}

	bool CameraComponent::Perspective::imGuiEditor() {
		bool edited = false;
		edited |= ImGui::SliderFloat("FOV", &mFOV, 0.f, 180.f);
		edited |= ImGui::SliderFloat("Aspect Ratio", &mAspectRatio, 0.f, 1.f);
		return edited;
	}

	bool CameraComponent::Orthographic::imGuiEditor() {
		bool edited = false;
		edited |= ImGui::SliderFloat2("H-Bounds", &mHorizBounds[0], -10.f, 10.f);
		edited |= ImGui::SliderFloat2("V-Bounds", &mVertBounds[0], -10.f, 10.f);
		return edited;
	}

}
