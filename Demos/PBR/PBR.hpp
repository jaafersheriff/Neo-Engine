#pragma once

#include "DemoInfra/IDemo.hpp"

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
		int mPointLightCount = 20;
		float mLightDebugRadius = 0.f;
		bool mDrawIBL = true;
		bool mDoTonemap = true;
		bool mDoBloom = true;
		float mBloomRadius = 0.005f;

		float mMinLuminance = 0.f;
		float mMaxLuminance = 10.f;
	};
}
