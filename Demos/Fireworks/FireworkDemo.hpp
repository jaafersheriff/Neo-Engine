#pragma once

#include "DemoInfra/IDemo.hpp"

#include "Renderer/RenderingSystems/AutoexposureRenderer.hpp"
#include "Renderer/RenderingSystems/BloomRenderer.hpp"

using namespace neo;

namespace Fireworks {

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
			-5.f,
			2.f,
			0.02f
		};


	};
}
