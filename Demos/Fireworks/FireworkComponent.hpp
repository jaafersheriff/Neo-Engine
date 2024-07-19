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
		glm::vec3 mChildColor = glm::vec3(1.f);
		float mChildColorBias = 0.4f;
	};

	FireworkComponent(const neo::ECS::Entity& entity, const neo::MeshManager& meshManager, uint32_t count);
	virtual void imGuiEditor() override;

	neo::MeshHandle mBuffer;
	uint32_t mCount;
	bool mNeedsInit;

	Parameters mParameters;

	END_COMPONENT();

}