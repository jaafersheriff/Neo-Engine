#pragma once

#include "ECS/Component/Component.hpp"

#include "ResourceManager/TextureManager.hpp"

namespace neo {
	class ECS;
	struct IBLConvolvedMessage;
	struct IBLDFGLutGeneratedMessage;

	START_COMPONENT(IBLComponent);
		// Filled in by the renderervia  IBLConvolvedMessage / IBLDFGLutGeneratedMessage
		TextureHandle mConvolvedSkybox = NEO_INVALID_HANDLE;
		bool mConvolved = false;
		uint16_t mConvolvedCubemapResolution = 512;
		uint16_t mSampleCount = 2048;

		TextureHandle mDFGLut = NEO_INVALID_HANDLE;
		bool mDFGGenerated = false;
		uint16_t mDFGLutResolution = 128;

		// Message handlign
		static void registerMessageHandlers(ECS& ecs);
		static void onConvolved(ECS& ecs, const IBLConvolvedMessage& message);
		static void onDFGLutGenerated(ECS& ecs, const IBLDFGLutGeneratedMessage& message);

		void imGuiEditor() {
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
