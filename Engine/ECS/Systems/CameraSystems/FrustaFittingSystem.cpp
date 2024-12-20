#include "ECS/pch.hpp"
#include "FrustaFittingSystem.hpp"

#include "ECS/ECS.hpp"
#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/CameraComponent/FrustumComponent.hpp"
#include "ECS/Component/CameraComponent/FrustumFitReceiverComponent.hpp"
#include "ECS/Component/CameraComponent/FrustumFitSourceComponent.hpp"
#include "ECS/Component/LightComponent/DirectionalLightComponent.hpp"
#include "ECS/Component/LightComponent/LightComponent.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"

namespace neo {

	void FrustaFittingSystem::update(ECS& ecs) {
		TRACY_ZONE();

		auto sourceCameraTuple = ecs.getSingleView<FrustumFitSourceComponent, SpatialComponent, CameraComponent>();
		auto receiverCameraTuple = ecs.getSingleView<FrustumFitReceiverComponent, SpatialComponent, CameraComponent, DirectionalLightComponent>();
		if (!receiverCameraTuple || !sourceCameraTuple) {
			return;
		}
		auto&& [sourceCameraEntity, _, sourceSpatial, sourceCamera] = *sourceCameraTuple;
		NEO_ASSERT(sourceCamera.getType() == CameraComponent::CameraType::Perspective, "Frustum fit source needs to be perspective");
		auto&& [receiverCameraEntity, receiverFrustum, receiverSpatial, receiverCamera, __] = *receiverCameraTuple;
		NEO_ASSERT(receiverCamera.getType() == CameraComponent::CameraType::Orthographic, "Frustum fit receiver needs to be orthographic");

		/////////////////////// Do the fitting! ///////////////////////////////

		const auto& sourceView = sourceSpatial.getView();
		const auto& sourceProj = sourceCamera.getProj();
		const auto& worldToLight = receiverSpatial.getView();
		const auto& lightToWorld = glm::inverse(worldToLight);

		// const float aspect = sourceProj[1][1] / sourceProj[0][0];
		// const float fov = 2.f * glm::atan(1.f / sourceProj[1][1]);
		// const float zNear = sourceProj[3][2] / (sourceProj[2][2] - 1.f);
		// const float zFar = sourceProj[3][2] / (sourceProj[2][2] + 1.f);
		// const float zRange = zFar - zNear;
		const float depthMin = -1.f; // GL things

		struct BoundingBox {
			glm::vec3 mMin = glm::vec3(std::numeric_limits<float>::max());
			glm::vec3 mMax = glm::vec3(-std::numeric_limits<float>::max());

			BoundingBox() {}

			BoundingBox(const FrustumComponent* bounds) {
				addNewPosition(bounds->mNearLeftBottom);
				addNewPosition(bounds->mNearLeftTop);
				addNewPosition(bounds->mNearRightBottom);
				addNewPosition(bounds->mNearRightTop);
				addNewPosition(bounds->mFarLeftBottom);
				addNewPosition(bounds->mFarLeftTop);
				addNewPosition(bounds->mFarRightBottom);
				addNewPosition(bounds->mFarRightTop);

			}

			void addNewPosition(glm::vec3 other) {
				if (other.x < mMin.x) { mMin.x = other.x; }
				if (other.y < mMin.y) { mMin.y = other.y; }
				if (other.z < mMin.z) { mMin.z = other.z; }
				if (other.x > mMax.x) { mMax.x = other.x; }
				if (other.y > mMax.y) { mMax.y = other.y; }
				if (other.z > mMax.z) { mMax.z = other.z; }
			}

			glm::vec3 center() {
				return glm::mix(mMin, mMax, 0.5f);
			}

			float width() {
				return mMax.x - mMin.x;
			}

			float height() {
				return mMax.y - mMin.y;
			}

			float depth() {
				return mMax.z - mMin.z;
			}
		};

		static const std::vector<glm::vec4> corners = { // screen space receiver box 
			{ -1.f,  1.f, depthMin, 1.f }, // corners of near plane
			{  1.f,  1.f, depthMin, 1.f },
			{ -1.f, -1.f, depthMin, 1.f },
			{  1.f, -1.f, depthMin, 1.f },
			{ -1.f,  1.f,	  1.f, 1.f }, // corners of far plane
			{  1.f,  1.f,	  1.f, 1.f },
			{ -1.f, -1.f,	  1.f, 1.f },
			{  1.f, -1.f,	  1.f, 1.f }
		};

		glm::mat4 shadowToWorld = glm::inverse(sourceProj * sourceView); // source view pos
		BoundingBox receiverBox;
		for (const auto& corner : corners) {
			glm::vec4 worldPos = shadowToWorld * corner; // transform corners of screen space unit receiver box into source PV space
			worldPos = worldPos / worldPos.w;
			worldPos = worldToLight * worldPos; // rotate corners with light's world rotation
			receiverBox.addNewPosition(worldPos);
		}

		glm::vec3 center = lightToWorld * glm::vec4(receiverBox.center(), 1.f); // receivers center back in light space
		float bias = receiverFrustum.mBias;
		const float boxWidth = receiverBox.width() * 0.5f;
		const float boxHeight = receiverBox.height() * 0.5f;
		const float boxDepth = receiverBox.depth() * 0.5f * (1.f + bias);

		receiverSpatial.setPosition(center); // Doesn't matter b/c it's an analytic directional light
		receiverCamera.setOrthographic(CameraComponent::Orthographic{ glm::vec2(-boxWidth, boxWidth), glm::vec2(-boxHeight, boxHeight) });
		float distToCenter = glm::distance(receiverSpatial.getPosition(), center);
		receiverCamera.setNear(distToCenter - boxDepth);
		receiverCamera.setFar(distToCenter + boxDepth);

	}
}
