#pragma once

#include "DemoInfra/IDemo.hpp"

#include "Renderer/RenderingSystems/PointLightShadowMapRenderer.hpp"
#include "Renderer/RenderingSystems/AutoexposureRenderer.hpp"
#include "Renderer/RenderingSystems/BloomRenderer.hpp"
#include "GBufferRenderer.hpp"

using namespace neo;

namespace DeferredPBR {

	class Demo : public IDemo {
	public:
		Demo() = default;
		virtual IDemo::Config getConfig() const override;
		virtual void init(ECS& ecs, ResourceManagers& resourceManagers) override;
		virtual void render(RenderPasses& renderPasses, const ResourceManagers& resourceManagers, const ECS& ecs, const TextureHandle& outputColor, const TextureHandle& outputDepth) override;
		virtual void destroy() override;
		virtual void imGuiEditor(ECS& ecs, ResourceManagers& resourceManagers) override;

	private:
		bool mDrawDirectionalShadows = true;
		bool mDrawPointLightShadows = true;

		int mPointLightCount = 2;
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
	};
}
