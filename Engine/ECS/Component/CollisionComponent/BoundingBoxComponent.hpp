#pragma once

#include "ECS/Component/Component.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"

#include "Loader/Loader.hpp"

namespace neo {

	START_COMPONENT(BoundingBoxComponent);

		bool mStatic = false;
		glm::vec3 mMin = glm::vec3(FLT_MAX);
		glm::vec3 mMax = glm::vec3(FLT_MIN);

		BoundingBoxComponent(bool isStatic = false) 
			: mMin(FLT_MAX)
			, mMax(FLT_MIN)
			, mStatic(isStatic)
		{ }

		BoundingBoxComponent(glm::vec3 min, glm::vec3 max, bool isStatic = false) 
			: mMin(min)
			, mMax(max)
			, mStatic(isStatic)
		{ }

		void addPoint(const glm::vec3& point) {
			mMin.x = glm::min(mMin.x, point.x);
			mMin.y = glm::min(mMin.y, point.y);
			mMin.z = glm::min(mMin.z, point.z);

			mMax.x = glm::max(mMax.x, point.x);
			mMax.y = glm::max(mMax.y, point.y);
			mMax.z = glm::max(mMax.z, point.z);
		}

		float getRadius() const {
			return glm::distance(mMin, mMax) / 2.f;
		}

		glm::vec3 getCenter() const {
			return mMin + ((mMax - mMin) / 2.f);
		}

		bool intersect(const glm::mat4& modelMatrix, const glm::vec3& position) const {
			return glm::length(glm::vec3(glm::inverse(modelMatrix) * glm::vec4(position, 1.f))) < getRadius();
		}

		std::optional<float> intersect(const glm::mat4 modelMatrix, const glm::vec3& rayPos, const glm::vec3& rayDir) const {
			// Bounding box in world space
			glm::vec3 min(modelMatrix * glm::vec4(mMin, 1.f));
			glm::vec3 max(modelMatrix * glm::vec4(mMax, 1.f));

			glm::vec3 tMin = (min - rayPos) / rayDir;
			glm::vec3 tMax = (max - rayPos) / rayDir;

			if (tMin.x > tMax.x) std::swap(tMin.x, tMax.x);
			if (tMin.y > tMax.y) std::swap(tMin.y, tMax.y);
			if (tMin.z > tMax.z) std::swap(tMin.z, tMax.z);

			// Ensure tMin is greater than 0 to ignore bounding boxes that encapsulate the ray's origin
			if (tMin.x < 0) tMin.x = 0;
			if (tMin.y < 0) tMin.y = 0;
			if (tMin.z < 0) tMin.z = 0;

			float tMinMax = glm::max(glm::max(tMin.x, tMin.y), tMin.z);
			float tMaxMin = glm::min(glm::min(tMax.x, tMax.y), tMax.z);

			if (tMaxMin >= tMinMax) {
				return tMinMax;
			}
			return std::nullopt; // No intersection
		}
	END_COMPONENT();
}