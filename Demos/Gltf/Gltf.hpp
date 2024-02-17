#pragma once

#include "DemoInfra/IDemo.hpp"

using namespace neo;

namespace Gltf {
	enum class DebugMode : uint8_t {
		Off,
		MetalRoughness,
		Emissives,
		COUNT
	};

	class Demo : public IDemo {
	public:
		virtual IDemo::Config getConfig() const override;
		virtual void init(ECS& ecs) override;
		virtual void update(ECS& ecs) override;
		virtual void render(const ECS& ecs, Framebuffer& backbuffer) override;
		virtual void destroy() override;
		virtual void imGuiEditor(ECS& ecs) override;

	private:
		bool mDoOcclusionMap = true;
		DebugMode mDebugMode = DebugMode::Off;
	};
}
