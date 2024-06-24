#pragma once

#include "ECS/Component/Component.hpp"

#include "ResourceManager/TextureManager.hpp"

namespace neo {
	START_COMPONENT(IBLComponent);
		// Yikes -- these have to be mutable b/c they're set by the renderer which has a const ref to the ECS
		// Use messaging instead?
		mutable TextureHandle mConvolvedSkybox = NEO_INVALID_HANDLE;
		mutable bool mConvolved = false;
		uint16_t mConvolvedCubemapResolution = 512;
		uint16_t mSampleCount = 2048;

		mutable TextureHandle mDFGLut = NEO_INVALID_HANDLE;
		mutable bool mDFGGenerated = false;
		uint16_t mDFGLutResolution = 128;

		virtual void imGuiEditor() override {
			if (ImGui::Button("Regenerate DFG Lut")) {
				mDFGGenerated = false;
			}
			bool regen = false; 
			int sc = mSampleCount;
			if (ImGui::SliderInt("SampleCount", &sc, 0, 2048)) {
				regen = true;
				mSampleCount = static_cast<uint16_t>(sc);
			}
			regen |= ImGui::Button("Reconvolve cubemap");
			if (regen) {
				mConvolved = false;
			}
		};
	END_COMPONENT();
}