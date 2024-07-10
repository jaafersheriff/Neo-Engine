#pragma once

#include "DemoInfra/IDemo.hpp"

#include <string>
#include <vector>

namespace neo {

	class ECS;

	class DemoWrangler {
	public:
		DemoWrangler(int& idx, std::vector<IDemo*>& demos);

		IDemo::Config getConfig();
		IDemo* getCurrentDemo();
		const std::vector<IDemo*>& getDemos() { return mDemos; }

		void swap();
		void setForceReload();
		bool needsReload();
		void imGuiEditor(ECS& ecs, ResourceManagers& resourceManagers);
	private:
		int& mCurrentDemoIndex;
		std::vector<IDemo*>& mDemos;
		int mNextDemoIndex = 0;
		bool mForceReload = false;
	};
}
