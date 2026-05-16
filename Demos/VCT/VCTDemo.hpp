#pragma once

#include "DemoInfra/IDemo.hpp"

#include "Renderer/RenderingSystems/AutoexposureRenderer.hpp"

using namespace neo;

namespace VCT {

	class Demo : public IDemo {
	public:
		virtual IDemo::Config getConfig() const override;
		virtual void init(ECS& ecs, ResourceManagers& resourceManagers) override;
		virtual void update(ECS& ecs, ResourceManagers& resourceManagers) override;
		virtual void render(RenderPasses& renderPasses, const ResourceManagers& resourceManagers, const ECS& ecs, const TextureHandle& outputColor, const TextureHandle& outputDepth) override;
		virtual void imGuiEditor(ECS& ecs, ResourceManagers& resourceManagers) override;
		virtual void destroy() override;

	private:
		bool mDebugDraw = false;
		AutoExposureParameters mAutoExposureParams = {
			-8.f,
			1.f,
			0.02f
		};
	};
}
