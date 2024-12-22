#include "ECS/pch.hpp"
#include "CSMFittingSystem.hpp"

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
			const uint16_t shadowMapResolution,
			const float bias,
			SpatialComponent& receiverSpatial,
			CameraComponent& receiverCamera,
			CSMCameraComponent* csmCamera
		) {

			// Set base
			receiverSpatial.setPosition(lightSpatial.getPosition());
			receiverSpatial.setScale(lightSpatial.getScale());
			receiverSpatial.setLookDir(lightSpatial.getLookDir());

			// Get main scene's proj matrix for this cascade
			glm::mat4 sourceProj = sourceCamera.getProj();
			{
				CameraComponent sourceCopy = sourceCamera;
				float range = sourceCopy.getFar() - sourceCopy.getNear();

				float nearOffset, farOffset;
				// [0, 0.2]
				if (csmCamera->getLod() == 0) {
					nearOffset = 0.f;
					farOffset = range * 0.2f;
				}
				// [0.2, 0.5]
				else if (csmCamera->getLod() == 1) {
					nearOffset = range * 0.2f;
					farOffset = range * 0.5f;
				}
				// [0.5, 1.0]
				else if (csmCamera->getLod() == 2) {
					nearOffset = range * 0.5f;
					farOffset = range;
				}
				else {
					nearOffset = farOffset = -1.f;
					NEO_FAIL("Invalid LOD");
				}

				csmCamera->mSliceDepths.x = sourceCopy.getNear() + nearOffset;
				csmCamera->mSliceDepths.y = sourceCopy.getNear() + farOffset;

				sourceCopy.setNear(csmCamera->mSliceDepths.x);
				sourceCopy.setFar(csmCamera->mSliceDepths.y);
				sourceProj = sourceCopy.getProj();
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
			const float texelsPerUnit = (shadowMapResolution >> csmCamera->getLod()) / (radius * 2.f);
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

			const float biasedRadius = radius + bias;
			receiverCamera.setOrthographic(CameraComponent::Orthographic{ glm::vec2(-biasedRadius, biasedRadius), glm::vec2(-biasedRadius, biasedRadius) });
			receiverCamera.setNear(-biasedRadius);
			receiverCamera.setFar(biasedRadius);
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

		const auto& lightReceiver = std::get<1>(*lightTuple);
		const auto& lightSpatial = std::get<2>(*lightTuple);
		const auto& shadowMap = std::get<5>(*lightTuple);
		uint16_t shadowMapResolution = 1;
		if (resourceManagers.mTextureManager.isValid(shadowMap.mShadowMap)) {
			const auto& shadowTexture = resourceManagers.mTextureManager.resolve(shadowMap.mShadowMap);
			shadowMapResolution = std::max(shadowTexture.mWidth, shadowTexture.mHeight);
		}

		auto csmCamera0Tuple = ecs.getSingleView<SpatialComponent, CameraComponent, CSMCamera0Component>();
		auto csmCamera1Tuple = ecs.getSingleView<SpatialComponent, CameraComponent, CSMCamera1Component>();
		auto csmCamera2Tuple = ecs.getSingleView<SpatialComponent, CameraComponent, CSMCamera2Component>();
		NEO_ASSERT(csmCamera0Tuple && csmCamera1Tuple && csmCamera2Tuple, "CSM Camera's dont exist");
		auto& [cameraEntity0, cameraSpatial0, cameraCamera0, csmCamera0] = *csmCamera0Tuple;
		auto& [cameraEntity1, cameraSpatial1, cameraCamera1, csmCamera1] = *csmCamera1Tuple;
		auto& [cameraEntity2, cameraSpatial2, cameraCamera2, csmCamera2] = *csmCamera2Tuple;

		NEO_ASSERT(
			cameraCamera0.getType() == CameraComponent::CameraType::Orthographic 
			&& cameraCamera0.getType() == cameraCamera1.getType() 
			&& cameraCamera1.getType() == cameraCamera2.getType(), "Frustum fit receiver needs to be orthographic");

		_doFitting(sourceSpatial, sourceCamera, lightSpatial, shadowMapResolution, lightReceiver.mBias, cameraSpatial0, cameraCamera0, &csmCamera0);
		_doFitting(sourceSpatial, sourceCamera, lightSpatial, shadowMapResolution, lightReceiver.mBias, cameraSpatial1, cameraCamera1, &csmCamera1);
		_doFitting(sourceSpatial, sourceCamera, lightSpatial, shadowMapResolution, lightReceiver.mBias, cameraSpatial2, cameraCamera2, &csmCamera2);
	}
}
