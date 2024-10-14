#pragma once

#include "ECS/Systems/System.hpp"

#include "ECS/Component/CameraComponent/CameraControllerComponent.hpp"

namespace neo {

	struct SpatialComponent;

	class CameraControllerSystem : public System {

	public:
		CameraControllerSystem() :
			System("CameraController System")
		{}

		virtual void update(ECS& ecs, const ResourceManagers& resourceManagers) override;
		virtual void imguiEditor(ECS& ecs) override;

		float mSuperSpeed = 2.5f;
	protected:
		void _updateLook(const float dt, ECS&, CameraControllerComponent& controller, SpatialComponent& spatial);
		void _updatePosition(const float dt, ECS&, CameraControllerComponent& controller, SpatialComponent& spatial);
	};
}