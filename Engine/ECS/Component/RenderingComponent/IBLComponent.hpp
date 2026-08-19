#pragma once

#include "ECS/Component/Component.hpp"

#include "ResourceManager/TextureManager.hpp"

namespace neo {
	class ECS;
	struct IBLConvolvedMessage;
	struct IBLDFGLutGeneratedMessage;

	START_COMPONENT(IBLComponent);
		// Filled in by the renderer, but through IBLConvolvedMessage / IBLDFGLutGeneratedMessage
		// rather than a mutable write: the renderer only ever sees a clone of the ECS, so writing
		// here directly would be discarded when the clone is refilled. The renderer owns the texture
		// names; this just remembers what it was handed.
		TextureHandle mConvolvedSkybox = NEO_INVALID_HANDLE;
		bool mConvolved = false;
		uint16_t mConvolvedCubemapResolution = 512;
		uint16_t mSampleCount = 2048;

		TextureHandle mDFGLut = NEO_INVALID_HANDLE;
		bool mDFGGenerated = false;
		uint16_t mDFGLutResolution = 128;

		// Opted into world-mutation message handling: ConvolveRenderer works on a clone of the ECS, so
		// what it discovers has to travel back as a message. Wired up automatically the first time a
		// demo attaches an IBLComponent - see ECS::addComponent.
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
