#pragma once

#include "DemoInfra/IDemo.hpp"

using namespace neo;

namespace PBR {
	enum class DebugMode : uint8_t {
		Off,
		Albedo,
		MetalRoughness,
		Normals,
		Emissives,
		Diffuse,
		Specular,
		COUNT
	};

	class Demo : public IDemo {
	public:
		virtual IDemo::Config getConfig() const override;
		virtual void init(ECS& ecs, ResourceManagers& resourceManagers) override;
		virtual void render(const ResourceManagers& resourceManagers, const ECS& ecs, Framebuffer& backbuffer) override;
		virtual void destroy() override;
		virtual void imGuiEditor(ECS& ecs) override;

	private:
		bool mDrawShadows = true;
		DebugMode mDebugMode = DebugMode::Off;
		bool mDrawIBL = true;
	};
}
