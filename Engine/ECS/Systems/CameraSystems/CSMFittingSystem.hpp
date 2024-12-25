#pragma once

#include "ECS/Systems/System.hpp"

namespace neo {

	class CSMFittingSystem : public neo::System {

	public:

		CSMFittingSystem(float lambda = 0.5f) 
			: neo::System("CSM Fitting System") 
			, mLambda(lambda)
		{ }

		virtual void update(neo::ECS& ecs, const ResourceManagers& resourceManagers) override;
		virtual void imguiEditor(ECS&) override;

	private:
		float mLambda = 0.5f;
	};
}
