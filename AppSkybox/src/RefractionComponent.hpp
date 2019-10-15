#pragma once

#include "Component/Component.hpp"

using namespace neo;

class RefractionComponent : public Component {

    public:

        RefractionComponent(GameObject *go, float r = 0.5f) :
            Component(go),
            mRatio(r)
        {}

        float mRatio = 0.f;

        virtual void imGuiEditor() override {
            ImGui::SliderFloat("Refraction", &mRatio, 0.f, 1.f);
        }
};