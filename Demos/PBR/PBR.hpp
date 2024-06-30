#pragma once

#include "DemoInfra/IDemo.hpp"

#include "Renderer/RenderingSystems/PBRRenderer.hpp"

using namespace neo;

namespace PBR {

	class Demo : public IDemo {
	public:
		Demo() = default;
		virtual IDemo::Config getConfig() const override;
		virtual void init(ECS& ecs, ResourceManagers& resourceManagers) override;
		virtual void render(const ResourceManagers& resourceManagers, const ECS& ecs, Framebuffer& backbuffer) override;
		virtual void destroy() override;
		virtual void imGuiEditor(ECS& ecs) override;

	private:
		bool mDrawShadows = true;
		bool mDrawIBL = true;
		bool mDoTonemap = true;
		bool mDoBloom = true;
		float mBloomRadius = 0.005f;
	};
}
