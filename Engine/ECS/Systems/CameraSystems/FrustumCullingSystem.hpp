#pragma once

#include "ECS/Systems/System.hpp"

namespace neo {

	class FrustumCullingSystem : public System {

	public:
		FrustumCullingSystem() :
			System("FrustumCulling System")
		{}

		virtual void update(ECS& ecs) override;
		virtual void imguiEditor(ECS&) override;

	private:
		int mCulledCount = -1;
	};
}