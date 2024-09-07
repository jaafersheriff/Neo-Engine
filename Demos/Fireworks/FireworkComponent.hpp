#pragma once

#include "ECS/ECS.hpp"
#include "ECS/Component/Component.hpp"

#include "ResourceManager/MeshManager.hpp"

#include <glm/glm.hpp>

namespace Fireworks {

	START_COMPONENT(FireworkComponent);

	struct Parameters {
		float mBaseSpeed = 3.0;
		float mVelocityDecay = 0.007f;
		float mGravity = 4.98f;
		float mMinIntensity = 0.45f;
		bool mInfinite = true;

		float mParentSpeed = 5.5f;
		float mParentIntensityDecay = 0.025f;
		float mParentLength = 0.22f;

		int mChildren = 16;
		float mChildPositionOffset = 0.04f;
		float mChildIntensity = 5.f;
		float mChildVelocityBias = 0.4f;
		float mChildIntensityDecay = 0.05f;
		float mChildLength = 0.1f;
		glm::vec3 mChildColor = glm::vec3(1.f);
		float mChildColorBias = 0.4f;
	};

	FireworkComponent(const neo::MeshManager& meshManager, uint32_t count);
	virtual void imGuiEditor() override;

	neo::MeshHandle mBuffer;
	uint32_t mCount;

	Parameters mParameters;

	END_COMPONENT();

}