#pragma once

#include "ECS/ECS.hpp"
#include "Util/Profiler.hpp"

#include "ECS/Component/RenderingComponent/PhongShaderComponent.hpp"
#include "ECS/Component/RenderingComponent/OpaqueComponent.hpp"
#include "ECS/Component/RenderingComponent/AlphaTestComponent.hpp"

#include "ECS/Component/CameraComponent/OrthoCameraComponent.hpp"
#include "ECS/Component/CameraComponent/ShadowCameraComponent.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"
#include "ECS/Component/RenderingComponent/MaterialComponent.hpp"
#include "ECS/Component/CollisionComponent/CameraCulledComponent.hpp"
#include "ECS/Component/CameraComponent/OrthoCameraComponent.hpp"

#include "ECS/Component/LightComponent/LightComponent.hpp"
#include "ECS/Component/LightComponent/DirectionalLightComponent.hpp"
#include "ECS/Component/LightComponent/PointLightComponent.hpp"

#include "Renderer/GLObjects/SourceShader.hpp"
#include "Renderer/GLObjects/ResolvedShaderInstance.hpp"

namespace Sponza {

	Framebuffer& createGBuffer(glm::uvec2 targetSize) {
		return *Library::getPooledFramebuffer({ targetSize, {
			// Albedo
			TextureFormat {
				types::texture::Target::Texture2D,
				types::texture::InternalFormats::RGB16_F,
			},
			// World 
			// TODO - could do everything in view space to get rid of this
			TextureFormat {
				types::texture::Target::Texture2D,
				types::texture::InternalFormats::RGB16_F,
			},
			// Normals
			TextureFormat {
				types::texture::Target::Texture2D,
				types::texture::InternalFormats::RGB16_F,
			},
			// Depth
			TextureFormat {
				types::texture::Target::Texture2D,
				types::texture::InternalFormats::D16,
			}
		} }, "Gbuffer");
	}

	template<typename... CompTs>
	void drawGBuffer(const ECS& ecs, ECS::Entity cameraEntity, const ShaderDefines& inDefines = {}) {
		TRACY_GPU();

		ShaderDefines passDefines(inDefines);
		bool containsAlphaTest = false;
		MakeDefine(ALPHA_TEST);
		if constexpr ((std::is_same_v<AlphaTestComponent, CompTs> || ...)) {
			containsAlphaTest = true;
			passDefines.set(ALPHA_TEST);
			// Transparency sorting..for later
		//	 glEnable(GL_BLEND);
		//	 ecs.sort<AlphaTestComponent>([&cameraSpatial, &ecs](ECS::Entity entityLeft, ECS::Entity entityRight) {
		//		 auto leftSpatial = ecs.cGetComponent<SpatialComponent>(entityLeft);
		//		 auto rightSpatial = ecs.cGetComponent<SpatialComponent>(entityRight);
		//		 if (leftSpatial && rightSpatial) {
		//			 return glm::distance(cameraSpatial->getPosition(), leftSpatial->getPosition()) < glm::distance(cameraSpatial->getPosition(), rightSpatial->getPosition());
		//		 }
		//		 return false;
		//		 });
		}

		const glm::mat4 P = ecs.cGetComponentAs<CameraComponent, PerspectiveCameraComponent>(cameraEntity)->getProj();
		const auto& cameraSpatial = ecs.cGetComponent<SpatialComponent>(cameraEntity);

		ShaderDefines drawDefines(passDefines);
		const auto& view = ecs.getView<const GBufferShaderComponent, const MeshComponent, const MaterialComponent, const SpatialComponent, const CompTs...>();
		for (auto entity : view) {
			// VFC
			if (auto* culled = ecs.cGetComponent<CameraCulledComponent>(entity)) {
				if (!culled->isInView(ecs, entity, cameraEntity)) {
					continue;
				}
			}

			if (containsAlphaTest) {
				NEO_ASSERT(!ecs.has<OpaqueComponent>(entity), "Entity has opaque and alpha test component?");
			}

			drawDefines.reset();

			const auto& material = view.get<const MaterialComponent>(entity);
			MakeDefine(ALBEDO_MAP);
			MakeDefine(NORMAL_MAP);
			if (material.mAlbedoMap) {
				drawDefines.set(ALBEDO_MAP);
			}
			if (material.mNormalMap) {
				drawDefines.set(NORMAL_MAP);
			}

			auto& resolvedShader = view.get<const GBufferShaderComponent>(entity).getResolvedInstance(drawDefines);
			resolvedShader.bind();

			resolvedShader.bindUniform("albedo", material.mAlbedoColor);
			if (material.mAlbedoMap) {
				resolvedShader.bindTexture("albedoMap", *material.mAlbedoMap);
			}

			if (material.mNormalMap) {
				resolvedShader.bindTexture("normalMap", *material.mNormalMap);
			}

			// UBO candidates
			{
				resolvedShader.bindUniform("P", P);
				resolvedShader.bindUniform("V", cameraSpatial->getView());
			}

			const auto& drawSpatial = view.get<const SpatialComponent>(entity);
			resolvedShader.bindUniform("M", drawSpatial.getModelMatrix());
			resolvedShader.bindUniform("N", drawSpatial.getNormalMatrix());

			view.get<const MeshComponent>(entity).mMesh->draw();
		}

		if (containsAlphaTest) {
			// glDisable(GL_BLEND);
		}
	}
}
