#pragma once

#include "DemoInfra/IDemo.hpp"
#include <string>

namespace neo {

	class DemoWrangler {
	public:
		DemoWrangler(int& idx, std::vector<IDemo*>& demos);

		IDemo::Config getConfig();
		IDemo* getCurrentDemo();
		const std::vector<IDemo*>& getDemos() { return mDemos; }

		void swap();
		void reload(int newDemo);
		void setForceReload();
		bool needsReload();
	private:
		int& mCurrentDemoIndex;
		std::vector<IDemo*>& mDemos;
		int mNextDemoIndex = 0;
		bool mForceReload = false;
	};
}
