#pragma once

#include "ECS/ECS.hpp"
#include "ECS/Component/Component.hpp"

#include "ECS/Component/CollisionComponent/BoundingBoxComponent.hpp"

#include "ECS/Systems/CameraSystems/FrustumSystem.hpp"
#include "ECS/Systems/CameraSystems/FrustumCullingSystem.hpp"

namespace neo {

	START_COMPONENT(CameraCulledComponent);
		using CameraIDs = std::vector<ECS::Entity>;
		CameraCulledComponent(CameraIDs ids = {}) :
			mCameraIDs(ids)
		{}

		bool isInView(const ECS& ecs, ECS::Entity thisID, ECS::Entity cameraID) const {
			// Requires FrustumSystem and FrustumCullingSystem to be active
			if (ecs.has<BoundingBoxComponent>(thisID)) {
				return std::find(mCameraIDs.begin(), mCameraIDs.end(), cameraID) != mCameraIDs.end();
			}

			return true;
		}

		CameraIDs mCameraIDs;
	END_COMPONENT();
}