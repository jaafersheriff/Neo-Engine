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

#pragma optimize("", off)

namespace neo {

	namespace {
		glm::vec3 _quantize(glm::vec3& pos, float texelSize) {
			pos.x -= glm::mod(pos.x, texelSize);
			pos.y -= glm::mod(pos.y, texelSize);
			pos.z -= glm::mod(pos.z, texelSize);
			return pos;
		}
	}

	void FrustaFittingSystem::update(ECS& ecs, const ResourceManagers& resourceManagers) {
		NEO_UNUSED(resourceManagers);

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

		glm::mat4 sceneToWorld = glm::inverse(sourceProj * sourceView); // source view pos
		BoundingBox receiverBox;
		for (const auto& corner : corners) {
			glm::vec4 worldPos = sceneToWorld * corner; // transform corners of screen space unit receiver box into source PV space
			worldPos = worldPos / worldPos.w;
			worldPos = worldToLight * worldPos; // rotate corners with light's world rotation
			receiverBox.addNewPosition(worldPos);
		}

		// https://alextardif.com/shadowmapping.html
		const float radius = std::max(receiverBox.width(), std::max(receiverBox.height(), receiverBox.depth())) / 2.f;
		const float texelsPerUnit = 256.f / (radius * 2.f); 
		const glm::mat4 scalar(glm::scale(glm::mat4(1.f), glm::vec3(texelsPerUnit)));

		glm::mat4 quantizedWorldToLight = scalar * worldToLight;
		glm::mat4 quantizedLightToWorld = glm::inverse(quantizedWorldToLight);

		glm::vec3 worldSpaceCenter = lightToWorld * glm::vec4(receiverBox.center(), 1.f);
		glm::vec3 quantizedLightSpaceCenter = quantizedWorldToLight * glm::vec4(worldSpaceCenter, 1.f);
		quantizedLightSpaceCenter.x = glm::floor(quantizedLightSpaceCenter.x);
		quantizedLightSpaceCenter.y = glm::floor(quantizedLightSpaceCenter.y);
		glm::vec3 quantizedWorldSpaceCenter = quantizedLightToWorld * glm::vec4(quantizedLightSpaceCenter, 1.f);

		glm::vec3 eye = quantizedWorldSpaceCenter - (receiverSpatial.getLookDir() * radius * 2.f);
		glm::mat4 finalLightView = glm::lookAt(eye, quantizedWorldSpaceCenter, glm::vec3(0, 1, 0));

		receiverSpatial.setOrientation(finalLightView);
		receiverSpatial.setPosition(quantizedWorldSpaceCenter);

		float finalRadius = std::floor(radius * texelsPerUnit) / texelsPerUnit; // This is close -- maybe I should do it in world space
		NEO_LOG_V("[Texels per unit] Original -- Final: [%0.3f] %0.3f -- %0.3f", texelsPerUnit, radius, finalRadius);
		receiverCamera.setOrthographic(CameraComponent::Orthographic{ glm::vec2(-finalRadius, finalRadius), glm::vec2(-finalRadius, finalRadius) });
		receiverCamera.setNear(-finalRadius);
		receiverCamera.setFar(finalRadius);


		// // Quantize radius in light space
		// glm::vec3 boxBounds(receiverBox.width(), receiverBox.height(), receiverBox.depth());
		// float radius = std::max(boxBounds.x, std::max(boxBounds.y, boxBounds.z)) / 2.f;

		// receiverCamera.setOrthographic(CameraComponent::Orthographic{ glm::vec2(-radius, radius), glm::vec2(-radius, radius) });
		// receiverCamera.setNear(-radius);
		// receiverCamera.setFar(radius);

		// const glm::vec3 originalCenter(receiverBox.center());
		// // Quantize in light space
		// float texelsPerUnit = 256.f / (radius * 2.f); // Texels / box diameter
		// glm::vec3 center(originalCenter);
		// _quantize(center, texelsPerUnit);
		// center.z = originalCenter.z;
		// receiverSpatial.setPosition(lightToWorld * glm::vec4(center, 1.f)); // Doesn't matter b/c it's an analytic directional light
	}
}
