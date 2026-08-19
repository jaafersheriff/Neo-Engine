#pragma once

#include "ECS/Component/Component.hpp"

namespace neo {

	START_COMPONENT(FrustumFitReceiverComponent);
		FrustumFitReceiverComponent(float bias = 0.f)
			: mBias(bias)
		{}

		void imGuiEditor() {
			ImGui::SliderFloat("Bias", &mBias, 0.f, 10.f);
		}
		float mBias = 0.f;
	END_COMPONENT();
}