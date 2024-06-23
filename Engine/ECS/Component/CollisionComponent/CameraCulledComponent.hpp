#pragma once

#include "ECS/ECS.hpp"
#include "ECS/Component/Component.hpp"

#include "ECS/Component/CollisionComponent/BoundingBoxComponent.hpp"

#include "ECS/Systems/CameraSystems/FrustumSystem.hpp"
#include "ECS/Systems/CameraSystems/FrustumCullingSystem.hpp"

namespace neo {

	START_COMPONENT(CameraCulledComponent);
		CameraCulledComponent(std::vector<ECS::Entity> set = {}) :
			mCameraViews(set)
		{}

		bool isInView(const ECS& ecs, ECS::Entity thisID, ECS::Entity cameraID) const {
			if (ecs.isSystemEnabled<FrustumSystem>() && ecs.isSystemEnabled<FrustumCullingSystem>() && ecs.has<BoundingBoxComponent>(thisID)) {
				return std::find(mCameraViews.begin(), mCameraViews.end(), cameraID) != mCameraViews.end();
			}

			return true;
		}

		std::vector<ECS::Entity> mCameraViews;
	END_COMPONENT();
}