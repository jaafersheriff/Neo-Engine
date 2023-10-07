#pragma once

#include "ECS/Component/Component.hpp"

namespace neo {

    struct FrustumFitReceiverComponent : public Component {
        FrustumFitReceiverComponent(float bias = 0.f)
            : mBias(bias)
        {}

        virtual std::string getName() const override { return "FrustumFitReceiverComponent"; }
        virtual void imGuiEditor() override {
            ImGui::SliderFloat("Bias", &mBias, 0.f, 10.f);
        }
        float mBias = 0.f;
    };
}