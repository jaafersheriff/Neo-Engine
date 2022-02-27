#include "DemoWrangler.h"

#include "imgui.h"

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

    void DemoWrangler::setForceReload() { 
        mForceReload = true; 
    }

    bool DemoWrangler::needsReload() { 
        return mForceReload || mNextDemoIndex != mCurrentDemoIndex; 
    };
    
    void DemoWrangler::imGuiEditor() {
        ImGui::Begin("Demos");
        if (ImGui::BeginCombo("Demos", getCurrentDemo()->getConfig().name.c_str())) {
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
        ImGui::End();

    }
}