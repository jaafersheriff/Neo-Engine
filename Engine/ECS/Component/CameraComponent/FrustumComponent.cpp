#include "ECS/pch.hpp"

#include "FrustumComponent.hpp"

#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/CollisionComponent/BoundingBoxComponent.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"

namespace neo {

	namespace {
		// https://iquilezles.org/articles/frustumcorrect/
		inline bool _boxInsideOfPlane(const glm::vec4& plane, const glm::vec3& min, const glm::vec3& max) {
			return !
				(true
					&& glm::dot(plane, glm::vec4(min.x, min.y, min.z, 1.f)) < 0.f
					&& glm::dot(plane, glm::vec4(max.x, min.y, min.z, 1.f)) < 0.f
					&& glm::dot(plane, glm::vec4(min.x, max.y, min.z, 1.f)) < 0.f
					&& glm::dot(plane, glm::vec4(max.x, max.y, min.z, 1.f)) < 0.f
					&& glm::dot(plane, glm::vec4(min.x, min.y, max.z, 1.f)) < 0.f
					&& glm::dot(plane, glm::vec4(max.x, min.y, max.z, 1.f)) < 0.f
					&& glm::dot(plane, glm::vec4(min.x, max.y, max.z, 1.f)) < 0.f
					&& glm::dot(plane, glm::vec4(max.x, max.y, max.z, 1.f)) < 0.f
				);
		}
		inline bool _planeInsideOfBox(const glm::vec3& corner, const glm::vec3& min, const glm::vec3& max) {
			return !(
				true
				&& corner.x < max.x
				&& corner.x > min.x
				&& corner.y < max.y
				&& corner.y > min.y
				&& corner.z < max.z
				&& corner.z > min.z
				)
			;
		}
	}

	bool FrustumComponent::isInFrustum(const SpatialComponent& spatial, const BoundingBoxComponent& box) const {
		const glm::vec3 min = spatial.getModelMatrix() * glm::vec4(box.mMin, 1.f);
		const glm::vec3 max = spatial.getModelMatrix() * glm::vec4(box.mMax, 1.f);

		return true
			&& _boxInsideOfPlane(mLeft, min, max)
			&& _boxInsideOfPlane(mRight, min, max)
			&& _boxInsideOfPlane(mTop, min, max)
			&& _boxInsideOfPlane(mBottom, min, max)
			&& _boxInsideOfPlane(mNear, min, max)
			&& _boxInsideOfPlane(mFar, min, max)
			//&& _planeInsideOfBox(mNearLeftBottom, min, max)
			//&& _planeInsideOfBox(mNearLeftTop, min, max)
			//&& _planeInsideOfBox(mNearRightBottom, min, max)
			//&& _planeInsideOfBox(mNearRightTop, min, max)
			//&& _planeInsideOfBox(mFarLeftBottom, min, max)
			//&& _planeInsideOfBox(mFarLeftTop, min, max)
			//&& _planeInsideOfBox(mFarRightBottom, min, max)
			//&& _planeInsideOfBox(mFarRightTop, min, max)
			;
	}

	void FrustumComponent::calculateFrustum(const CameraComponent& camera, const SpatialComponent& cameraSpatial) {

		glm::vec3 P = cameraSpatial.getPosition();
		glm::vec3 v = glm::normalize(cameraSpatial.getLookDir());
		glm::vec3 up = glm::normalize(cameraSpatial.getUpDir());
		glm::vec3 w = glm::normalize(cameraSpatial.getRightDir());

		// Update frustum bounds for camera type
		glm::mat4 PV(1.f);
		if (camera.getType() == CameraComponent::CameraType::Orthographic) {
			PV = camera.getProj() * cameraSpatial.getView();
			glm::vec3 Cnear = P + v * camera.getNear();
			glm::vec3 Cfar = P + v * camera.getFar();

			mNearLeftTop = Cnear + (up * camera.getOrthographic().mVertBounds.y) + (w * camera.getOrthographic().mHorizBounds.x);
			mNearRightTop = Cnear + (up * camera.getOrthographic().mVertBounds.y) + (w * camera.getOrthographic().mHorizBounds.y);
			mNearLeftBottom = Cnear + (up * camera.getOrthographic().mVertBounds.x) + (w * camera.getOrthographic().mHorizBounds.x);
			mNearRightBottom = Cnear + (up * camera.getOrthographic().mVertBounds.x) + (w * camera.getOrthographic().mHorizBounds.y);
			mFarLeftTop = Cfar + (up * camera.getOrthographic().mVertBounds.y) + (w * camera.getOrthographic().mHorizBounds.x);
			mFarRightTop = Cfar + (up * camera.getOrthographic().mVertBounds.y) + (w * camera.getOrthographic().mHorizBounds.y);
			mFarLeftBottom = Cfar + (up * camera.getOrthographic().mVertBounds.x) + (w * camera.getOrthographic().mHorizBounds.x);
			mFarRightBottom = Cfar + (up * camera.getOrthographic().mVertBounds.x) + (w * camera.getOrthographic().mHorizBounds.y);

			mNear.x = PV[0][3] + PV[0][2];
			mNear.y = PV[1][3] + PV[1][2];
			mNear.z = PV[2][3] + PV[2][2];
			mNear.w = PV[3][3] + PV[3][2];
			// mNear /= glm::length(glm::vec3(mNear));
		}
		else {
			PV = camera.getProj() * cameraSpatial.getView();
			float nDis = camera.getNear();
			float fDis = camera.getFar();
			glm::vec3 Cnear = P + v * nDis;
			glm::vec3 Cfar = P + v * fDis;

			float fov = glm::radians(camera.getPerspective().mFOV);
			float ar = camera.getPerspective().mAspectRatio;
			float Hnear = 2 * glm::tan(fov / 2) * nDis;
			float Wnear = Hnear * ar;
			float Hfar = 2 * glm::tan(fov / 2) * fDis;
			float Wfar = Hfar * ar;
			mNearLeftTop = Cnear + (up * (Hnear / 2)) - (w * (Wnear / 2));
			mNearRightTop = Cnear + (up * (Hnear / 2)) + (w * (Wnear / 2));
			mNearLeftBottom = Cnear - (up * (Hnear / 2)) - (w * (Wnear / 2));
			mNearRightBottom = Cnear - (up * (Hnear / 2)) + (w * (Wnear / 2));
			mFarLeftTop = Cfar + (up * (Hfar / 2)) - (w * (Wfar / 2));
			mFarRightTop = Cfar + (up * (Hfar / 2)) + (w * (Wfar / 2));
			mFarLeftBottom = Cfar - (up * (Hfar / 2)) - (w * (Wfar / 2));
			mFarRightBottom = Cfar - (up * (Hfar / 2)) + (w * (Wfar / 2));

			mNear.x = PV[0][2];
			mNear.y = PV[1][2];
			mNear.z = PV[2][2];
			mNear.w = PV[3][2];
			// mNear /= glm::length(glm::vec3(mNear));
		}

		// Update planes
		mLeft.x = PV[0][3] + PV[0][0];
		mLeft.y = PV[1][3] + PV[1][0];
		mLeft.z = PV[2][3] + PV[2][0];
		mLeft.w = PV[3][3] + PV[3][0];
		// mLeft /= glm::length(glm::vec3(mLeft));

		mRight.x = PV[0][3] - PV[0][0];
		mRight.y = PV[1][3] - PV[1][0];
		mRight.z = PV[2][3] - PV[2][0];
		mRight.w = PV[3][3] - PV[3][0];
		// mRight /= glm::length(glm::vec3(mRight));

		mBottom.x = PV[0][3] + PV[0][1];
		mBottom.y = PV[1][3] + PV[1][1];
		mBottom.z = PV[2][3] + PV[2][1];
		mBottom.w = PV[3][3] + PV[3][1];
		// mBottom /= glm::length(glm::vec3(mBottom));

		mTop.x = PV[0][3] - PV[0][1];
		mTop.y = PV[1][3] - PV[1][1];
		mTop.z = PV[2][3] - PV[2][1];
		mTop.w = PV[3][3] - PV[3][1];
		// mTop /= glm::length(glm::vec3(mTop));

		mFar.x = PV[0][3] - PV[0][2];
		mFar.y = PV[1][3] - PV[1][2];
		mFar.z = PV[2][3] - PV[2][2];
		mFar.w = PV[3][3] - PV[3][2];
		// mFar /= glm::length(glm::vec3(mFar));
	}

}
