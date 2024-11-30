#include "ECS/pch.hpp"
#include "CSMFitting.hpp"

#include "ECS/ECS.hpp"
#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/CameraComponent/CSMCameraComponent.hpp"
#include "ECS/Component/CameraComponent/FrustumComponent.hpp"
#include "ECS/Component/CameraComponent/FrustumFitReceiverComponent.hpp"
#include "ECS/Component/CameraComponent/FrustumFitSourceComponent.hpp"
#include "ECS/Component/CollisionComponent/BoundingBoxComponent.hpp"
#include "ECS/Component/LightComponent/DirectionalLightComponent.hpp"
#include "ECS/Component/LightComponent/LightComponent.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"

#include "ECS/Component/RenderingComponent/ShadowMapComponents.hpp"
#include "ResourceManager/ResourceManagers.hpp"

namespace neo {

	namespace {


		// https://alextardif.com/shadowmapping.html
		void _doFitting(
			const SpatialComponent& sourceSpatial,
			const CameraComponent& sourceCamera,
			const SpatialComponent& lightSpatial,
			SpatialComponent& receiverSpatial,
			CameraComponent& receiverCamera,
			const uint16_t shadowMapResolution,
			CSMCameraComponent& csmCamera
		) {

			// Set base
			receiverSpatial.setPosition(lightSpatial.getPosition());
			receiverSpatial.setScale(lightSpatial.getScale());
			receiverSpatial.setLookDir(lightSpatial.getLookDir());

			// Get main scene's proj matrix for this cascade
			glm::mat4 sourceProj = sourceCamera.getProj();
			{
				float depthLength = sourceCamera.getFar() - sourceCamera.getNear();
				float sliceDepth = depthLength / 4; // TODO - use log2

				float newNear = sourceCamera.getNear() + sliceDepth * csmCamera.getLod();
				float newFar = newNear + sliceDepth;
				CameraComponent sourceCopy = sourceCamera;
				sourceCopy.setNear(newNear);
				sourceCopy.setFar(newFar);

				sourceProj = sourceCopy.getProj();
				csmCamera.mSliceDepthEnd = newFar; // Use this in shadow resolve to compute which cascade to sample
			}

			const float depthMin = -1.f; // GL things
			static const std::vector<glm::vec3> corners = { // screen space receiver box 
				{ -1.f,  1.f, depthMin }, // corners of near plane
				{  1.f,  1.f, depthMin },
				{  1.f, -1.f, depthMin },
				{ -1.f, -1.f, depthMin },
				{ -1.f,  1.f,	   1.f }, // corners of far plane
				{  1.f,  1.f,	   1.f },
				{  1.f, -1.f,	   1.f },
				{ -1.f, -1.f,	   1.f }
			};

			const glm::mat4 iPV = glm::inverse(sourceProj * sourceSpatial.getView());

			BoundingBoxComponent frustumWorldBB;
			for (auto& corner : corners) {
				glm::vec4 tCorner = iPV * glm::vec4(corner, 1.f);
				frustumWorldBB.addPoint(tCorner / tCorner.w);
			}

			const float radius = glm::ceil(frustumWorldBB.getRadius());
			const float texelsPerUnit = (shadowMapResolution >> csmCamera.getLod()) / (radius * 2.f);
			const glm::mat4 scalar = glm::scale(glm::mat4(1.f), glm::vec3(texelsPerUnit));
			const glm::mat4 lookAt = scalar * lightSpatial.getView();
			const glm::mat4 iLookAt = glm::inverse(lookAt);

			glm::vec4 frustumCenter = lookAt * glm::vec4(frustumWorldBB.getCenter(), 1.f);
			frustumCenter /= frustumCenter.w;
			frustumCenter.x = glm::floor(frustumCenter.x);
			frustumCenter.y = glm::floor(frustumCenter.y);
			frustumCenter = iLookAt * glm::vec4(glm::vec3(frustumCenter), 1.f);
			frustumCenter /= frustumCenter.w;

			receiverSpatial.setPosition(frustumCenter);
			receiverSpatial.setOrientation(glm::lookAt(
				glm::vec3(frustumCenter), 
				glm::vec3(frustumCenter) + (lightSpatial.getLookDir() * radius * 2.f), 
				glm::vec3(0.f, 1.f, 0.f)
			));

			receiverCamera.setOrthographic(CameraComponent::Orthographic{ glm::vec2(-radius, radius), glm::vec2(-radius, radius) });
			receiverCamera.setNear(-radius);
			receiverCamera.setFar(radius);
		}
	}

	void CSMFittingSystem::update(ECS& ecs, const ResourceManagers& resourceManagers) {
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
