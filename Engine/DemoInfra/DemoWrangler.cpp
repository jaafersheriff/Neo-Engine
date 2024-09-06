#include "DemoWrangler.hpp"

#include "ext/imgui_incl.hpp"

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
		NEO_LOG_I("Swapping to demo %s", mDemos[mNextDemoIndex]->getConfig().name.c_str());
		mCurrentDemoIndex = mNextDemoIndex; mForceReload = false; 
	}

	void DemoWrangler::setForceReload() { 
		mForceReload = true; 
	}

	bool DemoWrangler::needsReload() { 
		return mForceReload || mNextDemoIndex != mCurrentDemoIndex; 
	};
	
	void DemoWrangler::imGuiEditor(ECS& ecs, ResourceManagers& resourceManagers) {
		ImGui::Begin("Demos");
		if (ImGui::BeginCombo("##Demos", getCurrentDemo()->getConfig().name.c_str())) {
			for (int i = 0; i < getDemos().size(); i++) {
				if (ImGui::Selectable(mDemos[i]->getConfig().name.c_str())) {
					mNextDemoIndex = i;
				}
			}
			ImGui::EndCombo();
		}
		if (ImGui::Button("Force reload")) {
			setForceReload();
		}

		ImGui::Separator();
		ImGui::Separator();
		getCurrentDemo()->imGuiEditor(ecs, resourceManagers);
		ImGui::End();

	}
}
