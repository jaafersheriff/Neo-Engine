#include "ECS/pch.hpp"

#include "FrustumComponent.hpp"

namespace neo {

	namespace {
		inline float _distanceToPlane(glm::vec4 plane, glm::vec3 position) {
			return plane.x * position.x + plane.y * position.y + plane.z * position.z + plane.w;
		}

		// https://iquilezles.org/articles/frustumcorrect/
		inline bool _boxInsideOfPlane(glm::vec4 plane, glm::vec3 min, glm::vec3 max) {
			return !
				(glm::dot(plane, glm::vec4(min.x, min.y, min.z, 1.f)) < 0.f
					&& glm::dot(plane, glm::vec4(max.x, min.y, min.z, 1.f)) < 0.f
					&& glm::dot(plane, glm::vec4(min.x, max.y, min.z, 1.f)) < 0.f
					&& glm::dot(plane, glm::vec4(max.x, max.y, min.z, 1.f)) < 0.f
					&& glm::dot(plane, glm::vec4(min.x, min.y, max.z, 1.f)) < 0.f
					&& glm::dot(plane, glm::vec4(max.x, min.y, max.z, 1.f)) < 0.f
					&& glm::dot(plane, glm::vec4(min.x, max.y, max.z, 1.f)) < 0.f
					&& glm::dot(plane, glm::vec4(max.x, max.y, max.z, 1.f)) < 0.f
					);
		}
	}

	// Test if an object is inside the frustum
	bool FrustumComponent::isInFrustum(const SpatialComponent& spatial, const BoundingBoxComponent& box) const {
		glm::vec3 min = box.mMin * spatial.getScale() + spatial.getPosition();
		glm::vec3 max = box.mMax * spatial.getScale() + spatial.getPosition();

		return true
			&& _boxInsideOfPlane(mLeft, min, max)
			&& _boxInsideOfPlane(mRight, min, max)
			&& _boxInsideOfPlane(mTop, min, max)
			&& _boxInsideOfPlane(mBottom, min, max)
			&& _boxInsideOfPlane(mNear, min, max)
			&& _boxInsideOfPlane(mFar, min, max)
			;
	}

}
