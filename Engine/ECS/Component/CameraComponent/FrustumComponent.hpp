#pragma once

#include "ECS/Component/Component.hpp"

#include "ECS/Component/CollisionComponent/BoundingBoxComponent.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"

namespace neo {

	START_COMPONENT(FrustumComponent);

		// Bounds
		glm::vec3 NearLeftBottom{ 0.f, 0.f, 0.f };
		glm::vec3 NearLeftTop{ 0.f, 0.f, 0.f };
		glm::vec3 NearRightBottom{ 0.f, 0.f, 0.f };
		glm::vec3 NearRightTop{ 0.f, 0.f, 0.f };
		glm::vec3 FarLeftBottom{ 0.f, 0.f, 0.f };
		glm::vec3 FarLeftTop{ 0.f, 0.f, 0.f };
		glm::vec3 FarRightBottom{ 0.f, 0.f, 0.f };
		glm::vec3 FarRightTop{ 0.f, 0.f, 0.f };

		// Planes
		glm::vec4 mLeft{ 0.f, 0.f, 0.f, 0.f };
		glm::vec4 mRight{ 0.f, 0.f, 0.f, 0.f };
		glm::vec4 mTop{ 0.f, 0.f, 0.f, 0.f };
		glm::vec4 mBottom{ 0.f, 0.f, 0.f, 0.f };
		glm::vec4 mNear{ 0.f, 0.f, 0.f, 0.f };
		glm::vec4 mFar{ 0.f, 0.f, 0.f, 0.f };

		// Test if an object is inside the frustum
		bool isInFrustum(const SpatialComponent& spatial, const BoundingBoxComponent& box) const;
	END_COMPONENT();
}
