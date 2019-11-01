#pragma once

#include "Component/Component.hpp"

using namespace neo;

class MetaballsComponent : public Component {

public:
    std::vector<glm::vec3> mPositions;
    std::vector<float> mRadius;
    bool mDirty = true;

    MetaballsComponent(GameObject* go, int numBalls = 16) :
        Component(go) {
        mPositions.resize(16);
        mRadius.resize(16);
    }

    virtual void imGuiEditor() override {
        static int index = 0;
        if (ImGui::Button("Add")) {
            mPositions.push_back(glm::vec3(0.f));
            mRadius.push_back(0.2f);
            mDirty = true;
        }
        ImGui::Separator();

        if (mPositions.size()) {
            ImGui::SliderInt("Index", &index, 0, mPositions.size() - 1);
            if (ImGui::SliderFloat3("Position", &mPositions[index][0], -10.f, 10.f)) {
                mDirty = true;
            }
            if (ImGui::SliderFloat("Radius", &mRadius[index], 0.f, 10.f)) {
                mDirty = true;
            }
            if (ImGui::Button("Remove")) {
                mPositions.erase(mPositions.begin() + index);
                mRadius.erase(mRadius.begin() + index);
                if (mPositions.size() == 1) {
                    index = 0;
                }
                else if (index == mPositions.size() - 1) {
                    index--;
                }
                mDirty = true;
            }
        }
    }
};
