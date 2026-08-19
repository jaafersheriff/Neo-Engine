#pragma once

#include "ECS/Component/Component.hpp"

#include "Renderer/RenderingSystems/AutoexposureRenderer.hpp"
#include "Renderer/RenderingSystems/BloomRenderer.hpp"
#include "GBufferRenderer.hpp"

namespace DeferredPBR {

	// What this demo's ImGui editor writes and its render() reads. A component so the frame
	// clone carries it to the render thread - the editor touches the live ECS, render only ever
	// sees the copy.
	START_COMPONENT(DeferredPBRParamsComponent);
		bool mDrawDirectionalShadows = true;
		bool mDrawPointLightShadows = true;

		float mLightDebugRadius = 0.1f;

		bool mDrawIBL = true;

		bool mDoTonemap = true;
		AutoExposureParameters mAutoExposureParams = {
			0.45f,
			45.f,
			0.02f
		};

		bool mDoBloom = true;
		BloomParameters mBloomParams = {
			0.004f,
			3,
			30.f
		};

		GBufferDebugParameters mGbufferDebugParams {
			GBufferDebugParameters::DebugMode::Off
		};
	END_COMPONENT();
}
