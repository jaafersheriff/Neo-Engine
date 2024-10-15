#include "ECS/pch.hpp"
#include "CSMFitting.hpp"

#include "ECS/ECS.hpp"
#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/CameraComponent/CSMCameraComponent.hpp"
#include "ECS/Component/CameraComponent/FrustumComponent.hpp"
#include "ECS/Component/CameraComponent/FrustumFitReceiverComponent.hpp"
#include "ECS/Component/CameraComponent/FrustumFitSourceComponent.hpp"
#include "ECS/Component/RenderingComponent/ShadowMapComponents.hpp"
#include "ECS/Component/LightComponent/DirectionalLightComponent.hpp"
#include "ECS/Component/LightComponent/LightComponent.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"

#include "ResourceManager/ResourceManagers.hpp"

#pragma optimize("", off)

namespace neo {

	namespace {

		void _doFitting(
			const SpatialComponent& sourceSpatial, 
			const CameraComponent& sourceCamera, 
			const SpatialComponent& lightSpatial, 
			SpatialComponent& receiverSpatial, 
			CameraComponent& receiverCamera, 
			const uint16_t shadowMapResolution,
			CSMCameraComponent& csmCamera
		) {
			/////////////////////// Do the fitting! ///////////////////////////////

			const auto& sourceView = sourceSpatial.getView();

			auto sourceProj = sourceCamera.getProj();
			{
				auto depth = sourceCamera.getFar() - sourceCamera.getNear();
				auto sliceDepth = depth / 4;

				auto newNear = sourceCamera.getNear() + sliceDepth * csmCamera.getLod();
				auto newFar = newNear + sliceDepth;
				CameraComponent sourceCopy = sourceCamera;
				sourceCopy.setNear(newNear);
				sourceCopy.setFar(newFar);

				sourceProj = sourceCopy.getProj();
				csmCamera.mSliceDepthEnd = newFar; // Use this in shadow resolve to compute which cascade to sample
			}

			receiverSpatial.setPosition(lightSpatial.getPosition());
			receiverSpatial.setScale(lightSpatial.getScale());
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
			//float bias = 10.f; // receiverFrustum.mBias; TODO bring this back
			float shadowRes = static_cast<float>(shadowMapResolution >> csmCamera.getLod());
			float boxWidth = receiverBox.width();
			boxWidth = glm::round(boxWidth * shadowRes) / shadowRes;
			float boxHeight = receiverBox.height();
			boxHeight = glm::round(boxHeight * shadowRes) / shadowRes;
			float boxDepth = receiverBox.depth(); // TODO - bring back bias
			boxDepth = glm::round(boxDepth * shadowRes) / shadowRes;

			receiverSpatial.setPosition(center); // Doesn't matter b/c it's an analytic directional light
			receiverCamera.setOrthographic(CameraComponent::Orthographic{ glm::vec2(-boxWidth / 2.f, boxWidth / 2.f), glm::vec2(-boxHeight / 2.f, boxHeight / 2.f) });
			float distToCenter = glm::distance(receiverSpatial.getPosition(), center);
			receiverCamera.setNear(distToCenter - boxDepth / 2.f);
			receiverCamera.setFar(distToCenter + boxDepth / 2.f);

		}
	}

	void CSMFitting::update(ECS& ecs, const ResourceManagers& resourceManagers) {
		TRACY_ZONE();

		auto sourceCameraTuple = ecs.getSingleView<FrustumFitSourceComponent, SpatialComponent, CameraComponent>();
		auto lightTuple = ecs.getSingleView<FrustumFitReceiverComponent, SpatialComponent, CameraComponent, DirectionalLightComponent, CSMShadowMapComponent>();

		if (!lightTuple || !sourceCameraTuple) {
			return;
		}
		auto&& [sourceCameraEntity, _, sourceSpatial, sourceCamera] = *sourceCameraTuple;
		NEO_ASSERT(sourceCamera.getType() == CameraComponent::CameraType::Perspective, "Frustum fit source needs to be perspective");

		const auto& lightSpatial = std::get<2>(*lightTuple);
		const auto& shadowMap = std::get<5>(*lightTuple);
		uint16_t shadowMapResolution = 0;
		if (resourceManagers.mTextureManager.isValid(shadowMap.mShadowMap)) {
			const auto& shadowTexture = resourceManagers.mTextureManager.resolve(shadowMap.mShadowMap);
			shadowMapResolution = std::max(shadowTexture.mWidth, shadowTexture.mHeight);
		}

		//NEO_ASSERT(receiverCamera.getType() == CameraComponent::CameraType::Orthographic, "Frustum fit receiver needs to be orthographic");
		// TODO - this should have asserts
		if (auto csmCamera0 = ecs.getSingleView<SpatialComponent, CameraComponent, CSMCamera0Component>()) {
			auto& [__, cameraSpatial, cameraCamera, csmCamera] = *csmCamera0;
			_doFitting(sourceSpatial, sourceCamera, lightSpatial, cameraSpatial, cameraCamera, shadowMapResolution, reinterpret_cast<CSMCameraComponent&>(csmCamera));
		}
		if (auto csmCamera1 = ecs.getSingleView<SpatialComponent, CameraComponent, CSMCamera1Component>()) {
			auto& [__, cameraSpatial, cameraCamera, csmCamera] = *csmCamera1;
			_doFitting(sourceSpatial, sourceCamera, lightSpatial, cameraSpatial, cameraCamera, shadowMapResolution, reinterpret_cast<CSMCameraComponent&>(csmCamera));
		}
		if (auto csmCamera2 = ecs.getSingleView<SpatialComponent, CameraComponent, CSMCamera2Component>()) {
			auto& [__, cameraSpatial, cameraCamera, csmCamera] = *csmCamera2;
			_doFitting(sourceSpatial, sourceCamera, lightSpatial, cameraSpatial, cameraCamera, shadowMapResolution, reinterpret_cast<CSMCameraComponent&>(csmCamera));
		}
		if (auto csmCamera3 = ecs.getSingleView<SpatialComponent, CameraComponent, CSMCamera3Component>()) {
			auto& [__, cameraSpatial, cameraCamera, csmCamera] = *csmCamera3;
			_doFitting(sourceSpatial, sourceCamera, lightSpatial, cameraSpatial, cameraCamera, shadowMapResolution, reinterpret_cast<CSMCameraComponent&>(csmCamera));
		}

	}
}
