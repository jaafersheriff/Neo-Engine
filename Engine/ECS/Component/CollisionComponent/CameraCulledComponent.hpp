#pragma once

#include "ECS/ECS.hpp"
#include "ECS/Component/Component.hpp"

#include "ECS/Component/CollisionComponent/BoundingBoxComponent.hpp"

#include "ECS/Systems/CameraSystems/FrustumSystem.hpp"
#include "ECS/Systems/CameraSystems/FrustumCullingSystem.hpp"

#include <algorithm>
#include <array>

namespace neo {

	START_COMPONENT(CameraCulledComponent);
		static constexpr uint8_t kMaxCameras = 8;
		using CameraIDs = std::array<ECS::Entity, kMaxCameras>;

		CameraCulledComponent() = default;
		CameraCulledComponent(const CameraIDs& ids, uint8_t count)
			: mCameraIDs(ids)
			, mCameraCount(count)
		{}

		bool isInView(const ECS& ecs, ECS::Entity thisID, ECS::Entity cameraID) const {
			// Requires FrustumSystem and FrustumCullingSystem to be active
			if (ecs.has<BoundingBoxComponent>(thisID)) {
				const auto last = mCameraIDs.begin() + mCameraCount;
				return std::find(mCameraIDs.begin(), last, cameraID) != last;
			}

			return true;
		}

		CameraIDs mCameraIDs = {};
		// Only the first mCameraCount entries of mCameraIDs are meaningful.
		uint8_t mCameraCount = 0;
	END_COMPONENT();
}
