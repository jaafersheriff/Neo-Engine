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

		const float depthMin = -1.f; // GL things
		std::vector<glm::vec3> frustumCorners = { // screen space receiver box 
			{ -1.f,  1.f, depthMin }, // corners of near plane
			{  1.f,  1.f, depthMin },
			{  1.f, -1.f, depthMin },
			{ -1.f, -1.f, depthMin },
			{ -1.f,  1.f,	   1.f }, // corners of far plane
			{  1.f,  1.f,	   1.f },
			{  1.f, -1.f,	   1.f },
			{ -1.f, -1.f,	   1.f }
		};

		glm::mat4 PV = sourceCamera.getProj() * sourceSpatial.getView();
		glm::mat4 iPV = glm::inverse(PV);

		glm::vec3 frustumCenter(0.f);
		for (auto& corner : frustumCorners) {
			corner = iPV * glm::vec4(corner, 1.f);
			frustumCenter += corner;
		}
		frustumCenter /= 8.f;

		const float radius = glm::distance(frustumCorners[0], frustumCorners[6]) / 2.f;
		const float texelsPerUnit = 512.f / (radius * 2.f); // TODO - replace w/ proper texture size
		const glm::mat4 scalar = glm::scale(glm::mat4(1.f), glm::vec3(texelsPerUnit));

		// TODO - this can lead to a feedback loop..
		const glm::vec3 baseLookDir = receiverSpatial.getLookDir(); // TODO - negate?
		const glm::mat4 lookAt = scalar * glm::lookAt(glm::vec3(0.f), baseLookDir, glm::vec3(0.f, 1.f, 0.f));
		const glm::mat4 iLookAt = glm::inverse(lookAt);

		frustumCenter = lookAt * glm::vec4(frustumCenter, 1.f);
		frustumCenter.x = glm::floor(frustumCenter.x);
		frustumCenter.y = glm::floor(frustumCenter.y);
		frustumCenter = iLookAt * glm::vec4(frustumCenter, 1.f);

		// TODO - feedback loop....
		receiverSpatial.setLookDir(frustumCenter + (receiverSpatial.getLookDir() * radius * 2.f));
		receiverSpatial.setPosition(frustumCenter);

		receiverCamera.setOrthographic(CameraComponent::Orthographic{ glm::vec2(-radius, radius), glm::vec2(-radius, radius) });
		receiverCamera.setNear(-radius);
		receiverCamera.setFar(radius);
	}
}
