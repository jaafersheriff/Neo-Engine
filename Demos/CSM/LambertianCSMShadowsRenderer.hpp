#pragma once

#include "ECS/ECS.hpp"
#include "Util/Profiler.hpp"

#include "ECS/Component/RenderingComponent/AlphaTestComponent.hpp"
#include "ECS/Component/RenderingComponent/PhongRenderComponent.hpp"
#include "ECS/Component/RenderingComponent/MeshComponent.hpp"
#include "ECS/Component/RenderingComponent/TransparentComponent.hpp"

#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"
#include "ECS/Component/RenderingComponent/MaterialComponent.hpp"
#include "ECS/Component/CollisionComponent/CameraCulledComponent.hpp"

#include "ECS/Component/LightComponent/MainLightComponent.hpp"
#include "ECS/Component/LightComponent/LightComponent.hpp"
#include "ECS/Component/LightComponent/DirectionalLightComponent.hpp"
#include "ECS/Component/LightComponent/PointLightComponent.hpp"

#include "Renderer/GLObjects/SourceShader.hpp"
#include "Renderer/GLObjects/ResolvedShaderInstance.hpp"

#include "ResourceManager/ResourceManagers.hpp"

namespace CSM {

	using namespace neo;

	template<typename... CompTs>
	void drawCSMResolve(const ResourceManagers& resourceManagers, const ECS& ecs, const ECS::Entity cameraEntity, bool debugView, const ShaderDefines& inDefines = {}) {
		TRACY_GPU();

		auto shaderHandle = resourceManagers.mShaderManager.asyncLoad("CSM Resolve Shader", 
			SourceShader::ConstructionArgs{
				{ types::shader::Stage::Vertex, "csm/model.vert"},
				{ types::shader::Stage::Fragment, "csm/csm.frag" }
			}
		);
		if (!resourceManagers.mShaderManager.isValid(shaderHandle)) {
			return;
		}


		const glm::mat4 P = ecs.cGetComponent<CameraComponent>(cameraEntity)->getProj();
		const auto& cameraSpatial = ecs.cGetComponent<SpatialComponent>(cameraEntity);
		auto lightView = ecs.getSingleView<MainLightComponent, LightComponent, SpatialComponent>();
		if (!lightView) {
			return;
		}
		const auto& [lightEntity, ____, light, lightSpatial] = *lightView;

		ShaderDefines passDefines(inDefines);
		MakeDefine(DEBUG_VIEW);
		if (debugView) {
			passDefines.set(DEBUG_VIEW);
		}

		glm::mat4 L0, L1, L2, L3;
		float depth0, depth1, depth2, depth3;
		glm::mat4 mockPV;
		float mockNear;
		const bool shadowsEnabled = 
			ecs.has<DirectionalLightComponent>(lightEntity) 
			&& ecs.has<CameraComponent>(lightEntity) 
			&& ecs.has<CSMShadowMapComponent>(lightEntity) 
			&& resourceManagers.mTextureManager.isValid(ecs.cGetComponent<CSMShadowMapComponent>(lightEntity)->mShadowMap);
		MakeDefine(ENABLE_SHADOWS);
		if (shadowsEnabled) {
			passDefines.set(ENABLE_SHADOWS);
			static glm::mat4 biasMatrix(
				0.5f, 0.0f, 0.0f, 0.0f,
				0.0f, 0.5f, 0.0f, 0.0f,
				0.0f, 0.0f, 0.5f, 0.0f,
				0.5f, 0.5f, 0.5f, 1.0f);
			// TODO - this should have asserts
			if (auto csmCamera0 = ecs.getSingleView<SpatialComponent, CameraComponent, CSMCamera0Component>()) {
				const auto& [_, csmSpatial, csmCamera, csm] = *csmCamera0;
				L0 = biasMatrix * csmCamera.getProj() * csmSpatial.getView();
				depth0 = csm.mSliceDepthEnd;
			}
			if (auto csmCamera1 = ecs.getSingleView<SpatialComponent, CameraComponent, CSMCamera1Component>()) {
				const auto& [_, csmSpatial, csmCamera, csm] = *csmCamera1;
				L1 = biasMatrix * csmCamera.getProj() * csmSpatial.getView();
				depth1 = csm.mSliceDepthEnd;
			}
			if (auto csmCamera2 = ecs.getSingleView<SpatialComponent, CameraComponent, CSMCamera2Component>()) {
				const auto& [_, csmSpatial, csmCamera, csm] = *csmCamera2;
				L2 = biasMatrix * csmCamera.getProj() * csmSpatial.getView();
				depth2 = csm.mSliceDepthEnd;
			}
			if (auto csmCamera3 = ecs.getSingleView<SpatialComponent, CameraComponent, CSMCamera3Component>()) {
				const auto& [_, csmSpatial, csmCamera, csm] = *csmCamera3;
				L3 = biasMatrix * csmCamera.getProj() * csmSpatial.getView();
				depth3 = csm.mSliceDepthEnd;
			}

			auto mockView = ecs.getSingleView<MockCameraComponent, SpatialComponent, CameraComponent>();
			if (mockView) {
				mockPV = std::get<3>(*mockView).getProj() * std::get<2>(*mockView).getView();
				mockNear = std::get<3>(*mockView).getNear();
			}
		}

		ShaderDefines drawDefines(passDefines);
		// No transparency sorting on the view, because I'm lazy, and this is stinky phong renderer
		const auto& view = ecs.getView<const PhongRenderComponent, const MeshComponent, const MaterialComponent, const SpatialComponent, const CompTs...>();
		for (auto entity : view) {
			// VFC
			if (auto* culled = ecs.cGetComponent<CameraCulledComponent>(entity)) {
				if (!culled->isInView(ecs, entity, cameraEntity)) {
					continue;
				}
			}

			drawDefines.reset();

			const auto& material = view.get<const MaterialComponent>(entity);
			MakeDefine(ALBEDO_MAP);
			MakeDefine(NORMAL_MAP);

			if (resourceManagers.mTextureManager.isValid(material.mAlbedoMap)) {
				drawDefines.set(ALBEDO_MAP);
			}
			if (resourceManagers.mTextureManager.isValid(material.mNormalMap)) {
				drawDefines.set(NORMAL_MAP);
			}

			auto& resolvedShader = resourceManagers.mShaderManager.resolveDefines(shaderHandle, drawDefines);
			resolvedShader.bind();

			resolvedShader.bindUniform("albedo", material.mAlbedoColor);

			// UBO candidates
			{
				resolvedShader.bindUniform("P", P);
				resolvedShader.bindUniform("V", cameraSpatial->getView());
				resolvedShader.bindUniform("camPos", cameraSpatial->getPosition());
				resolvedShader.bindUniform("lightCol", light.mColor);
				resolvedShader.bindUniform("lightDir", -lightSpatial.getLookDir());
				if (shadowsEnabled) {
					// These could be an array tbh
					resolvedShader.bindUniform("mockPV", mockPV);
					resolvedShader.bindUniform("mockNear", mockNear);
					resolvedShader.bindUniform("L0", L0);
					resolvedShader.bindUniform("L1", L1);
					resolvedShader.bindUniform("L2", L2);
					resolvedShader.bindUniform("L3", L3);
					resolvedShader.bindUniform("depth0", depth0);
					resolvedShader.bindUniform("depth1", depth1);
					resolvedShader.bindUniform("depth2", depth2);
					resolvedShader.bindUniform("depth3", depth3);
					resolvedShader.bindTexture("shadowMap", resourceManagers.mTextureManager.resolve(ecs.cGetComponent<CSMShadowMapComponent>(lightEntity)->mShadowMap));
				}
			}

			const auto& drawSpatial = view.get<const SpatialComponent>(entity);
			resolvedShader.bindUniform("M", drawSpatial.getModelMatrix());
			resolvedShader.bindUniform("N", drawSpatial.getNormalMatrix());

			resourceManagers.mMeshManager.resolve(view.get<const MeshComponent>(entity).mMeshHandle).draw();
		}
	}
}