#pragma once

#include "DemoInfra/IDemo.hpp"

#include "Renderer/RenderingSystems/AutoexposureRenderer.hpp"
#include "Renderer/RenderingSystems/BloomRenderer.hpp"

using namespace neo;

namespace Fireworks {
	struct FireworkParameters {
		float mBaseSpeed = 3.0;
		int mParents = 5;
		float mParentIntensity = 100.f;
		float mParentSpeed = 4.5f;
		float mChildPositionOffset = 0.05f;
		float mChildIntensity = 0.65f;
		float mChildVelocityBias = 0.4f;
	};

	class Demo : public IDemo {
	public:
		virtual IDemo::Config getConfig() const override;
		virtual void init(ECS& ecs, ResourceManagers& resourceManagers) override;
		virtual void update(ECS& ecs, ResourceManagers& resourceManagers) override;
		virtual void render(const ResourceManagers& resourceManagers, const ECS& ecs, Framebuffer& backbuffer) override;
		virtual void destroy() override;
		virtual void imGuiEditor(ECS& ecs, ResourceManagers& resourceManagers) override;

	private:
		BloomParameters mBloomParams = {
			0.005f,
			4,
			0.f
		};
		AutoExposureParameters mAutoExposureParams = {
			-8.f,
			1.f,
			0.02f
		};

		FireworkParameters mFireworkParameters;
	};
}
