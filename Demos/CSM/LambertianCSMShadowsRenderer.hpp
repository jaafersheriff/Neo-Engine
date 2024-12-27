#pragma once

#include "ECS/ECS.hpp"
#include "Util/Profiler.hpp"

#include "ECS/Component/CameraComponent/CSMCameraComponent.hpp"

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
				{ types::shader::Stage::Vertex, "model.vert"},
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
		MakeDefine(ENABLE_SHADOWS);
		passDefines.set(ENABLE_SHADOWS);

		MakeDefine(DEBUG_VIEW);
		if (debugView) {
			passDefines.set(DEBUG_VIEW);
		}

		CSMShadowInfo csmShadowInfo = extractCSMShadowInfo(ecs, lightEntity, resourceManagers.mTextureManager);
		if (!csmShadowInfo.mValidCSMShadows) {
			return;
		}

		auto mockView = ecs.getSingleView<MockCameraComponent, SpatialComponent, CameraComponent>();
		if (!mockView) {
			NEO_FAIL("Heh?");
		}
		glm::mat4 mockPV = std::get<3>(*mockView).getProj() * std::get<2>(*mockView).getView();
		float mockNear = std::get<3>(*mockView).getNear();

		ShaderDefines drawDefines(passDefines);
		// No transparency sorting on the view, because I'm lazy, and this is stinky phong renderer
		const auto& view = ecs.getView<const MeshComponent, const MaterialComponent, const SpatialComponent, const CompTs...>();
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

				// These could be an array tbh
				resolvedShader.bindUniform("mockPV", mockPV);
				resolvedShader.bindUniform("mockNear", mockNear);
				resolvedShader.bindUniform("L0", csmShadowInfo.mLightArrays[0]);
				resolvedShader.bindUniform("L1", csmShadowInfo.mLightArrays[1]);
				resolvedShader.bindUniform("L2", csmShadowInfo.mLightArrays[2]);
				const auto shadowTexture = resourceManagers.mTextureManager.resolve(ecs.cGetComponent<CSMShadowMapComponent>(lightEntity)->mShadowMap);
				resolvedShader.bindTexture("shadowMap", shadowTexture);
				resolvedShader.bindUniform("shadowMapResolution", static_cast<float>(shadowTexture.mWidth));
			}

			const auto& drawSpatial = view.get<const SpatialComponent>(entity);
			resolvedShader.bindUniform("M", drawSpatial.getModelMatrix());
			resolvedShader.bindUniform("N", drawSpatial.getNormalMatrix());

			resourceManagers.mMeshManager.resolve(view.get<const MeshComponent>(entity).mMeshHandle).draw();
		}
	}
}