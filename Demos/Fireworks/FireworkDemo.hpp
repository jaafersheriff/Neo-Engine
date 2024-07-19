#pragma once

#include "DemoInfra/IDemo.hpp"

#include "Renderer/RenderingSystems/AutoexposureRenderer.hpp"
#include "Renderer/RenderingSystems/BloomRenderer.hpp"

using namespace neo;

namespace Fireworks {
	struct FireworkParameters {
		float mBaseSpeed = 3.0;
		float mVelocityDecay = 0.007f;
		float mGravity = 2.98f;
		float mMinIntensity = 0.15f;
		bool mInfinite = true;

		int mParents = 5;
		glm::vec3 mParentColor = glm::vec3(1.f, 0.25f, 0);
		float mParentIntensity = 6000.f;
		float mParentSpeed = 4.5f;
		float mParentIntensityDecay = 0.02f;
		float mParentLength = 0.22f;

		float mChildPositionOffset = 0.08f;
		float mChildIntensity = 0.3f;
		float mChildVelocityBias = 0.6f;
		float mChildIntensityDecay = 0.008f;
		float mChildLength = 0.4f;

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
