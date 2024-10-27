#include "ECS/pch.hpp"
#include "CSMFitting.hpp"

#include "ECS/ECS.hpp"
#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/CameraComponent/CSMCameraComponent.hpp"
#include "ECS/Component/CameraComponent/FrustumComponent.hpp"
#include "ECS/Component/CameraComponent/FrustumFitReceiverComponent.hpp"
#include "ECS/Component/CameraComponent/FrustumFitSourceComponent.hpp"
#include "ECS/Component/LightComponent/DirectionalLightComponent.hpp"
#include "ECS/Component/LightComponent/LightComponent.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"

#include "ECS/Component/RenderingComponent/ShadowMapComponents.hpp"
#include "ResourceManager/ResourceManagers.hpp"

namespace neo {

	namespace {
		void _quantize(glm::vec3& pos, float texelSize) {
			pos.x -= glm::mod(pos.x, texelSize);
			pos.y -= glm::mod(pos.y, texelSize);
			pos.z -= glm::mod(pos.z, texelSize);
		}

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
			receiverSpatial.setLookDir(lightSpatial.getLookDir());
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

			// https://alextardif.com/shadowmapping.html
			float radius = std::max(receiverBox.width(), std::max(receiverBox.height(), receiverBox.depth())) / 2.f;
			float textureSize = static_cast<float>(shadowMapResolution >> csmCamera.getLod());
			float texelsPerUnit = textureSize / (radius * 2.f);
			glm::mat4 scalar(glm::scale(glm::mat4(1.f), glm::vec3(texelsPerUnit)));

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


			float finalRadius = radius - glm::mod(radius, 1.f / 256.f);
			receiverCamera.setOrthographic(CameraComponent::Orthographic{ glm::vec2(-finalRadius, finalRadius), glm::vec2(-finalRadius, finalRadius) });
			receiverCamera.setNear(-finalRadius);
			receiverCamera.setFar(finalRadius);

			// float texelSize = 1.f / static_cast<float>(shadowMapResolution >> csmCamera.getLod());

			// glm::vec3 center = lightToWorld * glm::vec4(receiverBox.center(), 1.f); // receivers center back in light space
			// //float bias = 10.f; // receiverFrustum.mBias; TODO bring this back
			// glm::vec3 boxBounds(receiverBox.width(), receiverBox.height(), receiverBox.depth());
			// _quantize(boxBounds, texelSize);
			// float radius = std::max(boxBounds.x, std::max(boxBounds.y, boxBounds.z));
			// const float boxWidth = radius;
			// const float boxHeight = radius;
			// const float boxDepth = radius;

			// receiverSpatial.setPosition(center); // Doesn't matter b/c it's an analytic directional light
			// receiverCamera.setOrthographic(CameraComponent::Orthographic{ glm::vec2(-boxWidth * 0.5f, boxWidth * 0.5f), glm::vec2(-boxHeight * 0.5f, boxHeight * 0.5f) });
			// receiverCamera.setNear(-boxDepth * 0.5f);
			// receiverCamera.setFar(boxDepth * 0.5f);

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
		uint16_t shadowMapResolution = 1;
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
