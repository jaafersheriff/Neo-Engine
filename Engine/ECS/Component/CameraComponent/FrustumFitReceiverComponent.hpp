#pragma once

#include "ECS/Component/Component.hpp"

namespace neo {

    class FrustumFitReceiverComponent : public Component {
    public:
        FrustumFitReceiverComponent(GameObject *go, float bias = 0.f)
            : Component(go)
            , mBias(bias)
        {}

        virtual void imGuiEditor() override {
            ImGui::SliderFloat("Bias", &mBias, 0.f, 10.f);
        }
        float mBias = 0.f;
    };
}