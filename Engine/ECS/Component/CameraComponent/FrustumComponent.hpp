#pragma once

#include "ECS/Component/Component.hpp"


namespace neo {
	struct BoundingBoxComponent;
	struct SpatialComponent;
	struct CameraComponent;

	START_COMPONENT(FrustumComponent);

		// Bounds
		glm::vec3 mNearLeftBottom{ 0.f, 0.f, 0.f };
		glm::vec3 mNearLeftTop{ 0.f, 0.f, 0.f };
		glm::vec3 mNearRightBottom{ 0.f, 0.f, 0.f };
		glm::vec3 mNearRightTop{ 0.f, 0.f, 0.f };
		glm::vec3 mFarLeftBottom{ 0.f, 0.f, 0.f };
		glm::vec3 mFarLeftTop{ 0.f, 0.f, 0.f };
		glm::vec3 mFarRightBottom{ 0.f, 0.f, 0.f };
		glm::vec3 mFarRightTop{ 0.f, 0.f, 0.f };

		// Planes
		glm::vec4 mLeft{ 0.f, 0.f, 0.f, 0.f };
		glm::vec4 mRight{ 0.f, 0.f, 0.f, 0.f };
		glm::vec4 mTop{ 0.f, 0.f, 0.f, 0.f };
		glm::vec4 mBottom{ 0.f, 0.f, 0.f, 0.f };
		glm::vec4 mNear{ 0.f, 0.f, 0.f, 0.f };
		glm::vec4 mFar{ 0.f, 0.f, 0.f, 0.f };

		void calculateFrustum(const CameraComponent& camera, const SpatialComponent& cameraSpatial);

		// Test if an object is inside the frustum
		bool isInFrustum(const SpatialComponent& spatial, const BoundingBoxComponent& box) const;
	END_COMPONENT();
}
