#pragma once

#include "ECS/Component/Component.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"

#include "Loader/Loader.hpp"

namespace neo {

	START_COMPONENT(BoundingBoxComponent);

		bool mStatic = false;
		glm::vec3 mMin = glm::vec3(FLT_MAX);
		glm::vec3 mMax = glm::vec3(-FLT_MAX);

		BoundingBoxComponent(bool isStatic = false) 
			: mMin(FLT_MAX)
			, mMax(-FLT_MAX)
			, mStatic(isStatic)
		{ }

		BoundingBoxComponent(glm::vec3 min, glm::vec3 max, bool isStatic = false) 
			: mMin(min)
			, mMax(max)
			, mStatic(isStatic)
		{ }

		void addPoint(const glm::vec3& point) {
			mMin.x = std::min(mMin.x, point.x);
			mMin.y = std::min(mMin.y, point.y);
			mMin.z = std::min(mMin.z, point.z);

			mMax.x = std::max(mMax.x, point.x);
			mMax.y = std::max(mMax.y, point.y);
			mMax.z = std::max(mMax.z, point.z);
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

		bool intersect(const glm::mat4& modelMatrix, const BoundingBoxComponent& other, const glm::mat4& otherModelMatrix) const {
			glm::vec3 aMin = glm::vec3(modelMatrix * glm::vec4(mMin, 1.0));
			glm::vec3 aMax = glm::vec3(modelMatrix * glm::vec4(mMax, 1.0));
			glm::vec3 bMin = glm::vec3(otherModelMatrix * glm::vec4(other.mMin, 1.0));
			glm::vec3 bMax = glm::vec3(otherModelMatrix * glm::vec4(other.mMax, 1.0));

			// Axis overlap test in world space
			bool overlapX = (aMin.x <= bMax.x) && (aMax.x >= bMin.x);
			bool overlapY = (aMin.y <= bMax.y) && (aMax.y >= bMin.y);
			bool overlapZ = (aMin.z <= bMax.z) && (aMax.z >= bMin.z);
			return overlapX && overlapY && overlapZ;
		}
	END_COMPONENT();
}