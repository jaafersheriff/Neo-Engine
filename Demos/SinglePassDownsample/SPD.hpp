#pragma once

#include "DemoInfra/IDemo.hpp"

#include "DownsampleRenderSystem.hpp"

using namespace neo;

namespace SPD {

	class Demo : public IDemo {
	public:
		virtual IDemo::Config getConfig() const override;
		virtual void init(ECS& ecs, ResourceManagers& resourceManagers) override;
		virtual void update(ECS& ecs, ResourceManagers& resourceManagers) override;
		virtual void render(const ResourceManagers& resourceManagers, const ECS& ecs, Framebuffer& backbuffer) override;
		virtual void destroy() override;
		virtual void imGuiEditor(ECS& ecs, ResourceManagers& resourceManagers) override;

	private:
		TextureHandle mHiZTextureHandle = NEO_INVALID_HANDLE;
	};
}
