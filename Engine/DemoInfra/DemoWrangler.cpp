#include "DemoWrangler.h"

namespace neo {

	DemoWrangler::DemoWrangler(int& idx, std::vector<IDemo*>& demos)
		: mCurrentDemoIndex(idx)
		, mDemos(demos)
	{}

	IDemo::Config DemoWrangler::getConfig() { 
		return mDemos[mCurrentDemoIndex]->getConfig(); 
	}
	IDemo* DemoWrangler::getCurrentDemo() { 
		return mDemos[mCurrentDemoIndex]; 
	}

	void DemoWrangler::swap() { 
		mCurrentDemoIndex = mNextDemoIndex; mForceReload = false; 
	}

	void DemoWrangler::reload(int newDemo) { 
		mNextDemoIndex = newDemo; 
	}

	void DemoWrangler::setForceReload() { 
		mForceReload = true; 
	}

	bool DemoWrangler::needsReload() { 
		return mForceReload || mNextDemoIndex != mCurrentDemoIndex; 
	};
}