#pragma once

#include "DemoInfra/IDemo.hpp"

using namespace neo;

namespace Base {

	class Demo : public IDemo {
	public:
		virtual IDemo::Config getConfig() const override;
		virtual void init(ECS& ecs, MeshManager& meshManager) override;
		virtual void update(ECS& ecs, MeshManager& meshManager) override;
		virtual void render(const MeshManager& meshManager, const ECS& ecs, Framebuffer& backbuffer) override;
		virtual void destroy() override;
		virtual void imGuiEditor(ECS& ecs) override;

	};
}
