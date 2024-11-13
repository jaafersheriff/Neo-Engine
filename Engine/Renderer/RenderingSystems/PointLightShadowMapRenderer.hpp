#pragma once

#include "ECS/ECS.hpp"

#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/CameraComponent/FrustumComponent.hpp"
#include "ECS/Component/RenderingComponent/ShadowCasterRenderComponent.hpp"

#include "Renderer/GLObjects/SourceShader.hpp"
#include "Renderer/GLObjects/ResolvedShaderInstance.hpp"
#include "Renderer/GLObjects/Framebuffer.hpp"

#include "ResourceManager/ResourceManagers.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <inttypes.h>

namespace neo {

	template<typename... CompTs, typename... Deps>
	inline void drawPointLightShadows(
		FrameGraph& fg,
		FramebufferHandle outputTarget,
		const ResourceManagers& resourceManagers,
		const ECS& ecs,
		const ECS::Entity& lightEntity,
		uint8_t faceIndex,
		Deps... deps
	) {
		TRACY_ZONE();

		auto shaderHandle = resourceManagers.mShaderManager.asyncLoad("PointLightShadowMap Shader", SourceShader::ConstructionArgs{
			{ types::shader::Stage::Vertex, "model.vert"},
			{ types::shader::Stage::Fragment, "pointlightdepth.frag" }
			});

		Viewport vp(0);
		{
			if (resourceManagers.mFramebufferManager.isValid(outputTarget)) {
				const TextureHandle& textureHandle = resourceManagers.mFramebufferManager.resolve(outputTarget).mTextures[0];
				if (resourceManagers.mTextureManager.isValid(textureHandle)) {
					const Texture& shadowTexture = resourceManagers.mTextureManager.resolve(textureHandle);
					vp.z = vp.w = shadowTexture.mWidth;
				}
			}
		}
		NEO_ASSERT(ecs.has<PointLightComponent>(lightEntity) && ecs.has<ShadowCameraComponent>(lightEntity), "Invalid light entity for point ligth shadows");
		NEO_ASSERT(ecs.has<SpatialComponent>(lightEntity), "Point light shadows need a spatial");

		fg.pass(outputTarget, vp, vp, {}, shaderHandle)
			.with([lightEntity, faceIndex](Pass& pass, const ResourceManagers& resourceManagers, const ECS& ecs) {
			TRACY_ZONEN("drawPointLightShadows PassBuilder");
			SpatialComponent cameraSpatial = *ecs.cGetComponent<SpatialComponent>(lightEntity); // Copy
			FrustumComponent frustum;
			CameraComponent camera(
				0.2f,
				cameraSpatial.getScale().x / 2.f,
				CameraComponent::Perspective{ 90.f, 1.f }
			);
			static std::vector<std::vector<glm::vec3>> lookDirs = {
				{ glm::vec3(1, 0, 0), glm::vec3(0, -1, 0) },
				{ glm::vec3(-1, 0, 0), glm::vec3(0, -1, 0) },
				{ glm::vec3(0, 1, 0), glm::vec3(0,  0, 1) },
				{ glm::vec3(0,-1, 0), glm::vec3(0,  0,-1) },
				{ glm::vec3(0, 0, 1), glm::vec3(0, -1, 0) },
				{ glm::vec3(0, 0,-1), glm::vec3(0, -1, 0) }
			};

			cameraSpatial.setLookDir(lookDirs[faceIndex][0], lookDirs[faceIndex][1]);
			frustum.calculateFrustum(camera, cameraSpatial);

			const auto& view = ecs.getView<const ShadowCasterRenderComponent, const MeshComponent, const SpatialComponent, CompTs...>();
			for (auto entity : view) {
				const SpatialComponent& drawSpatial = view.get<const SpatialComponent>(entity);
				// VFC
				if (ecs.has<BoundingBoxComponent>(entity) && !frustum.isInFrustum(drawSpatial, *ecs.cGetComponent<BoundingBoxComponent>(entity))) {
					continue;
				}
				
				ShaderDefinesFG drawDefines;
				UniformBuffer ubo;

				auto material = ecs.cGetComponent<const MaterialComponent>(entity);

				MakeDefine(ALPHA_TEST);
				if constexpr ((std::is_same_v<AlphaTestComponent, CompTs> || ...)) {
					if (material && resourceManagers.mTextureManager.isValid(material->mAlbedoMap)) {
						drawDefines.set(ALPHA_TEST);
						ubo.bindTexture("alphaMap", material->mAlbedoMap);
					}
				}

				ubo.bindUniform("P", camera.getProj());
				ubo.bindUniform("V", cameraSpatial.getView());
				ubo.bindUniform("lightPos", cameraSpatial.getPosition());
				ubo.bindUniform("lightRange", (cameraSpatial.getScale().x - 0.5) / 2.f);

				ubo.bindUniform("M", drawSpatial.getModelMatrix());
				pass.drawCommand(view.get<const MeshComponent>(entity).mMeshHandle, ubo, drawDefines);
			}
		})
		.dependsOn(std::forward<Deps>(deps)...)
		.setDebugName("PointLightShadows");
	}
}
