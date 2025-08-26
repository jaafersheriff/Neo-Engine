#pragma once

#include "ECS/ECS.hpp"

#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/CameraComponent/FrustumComponent.hpp"
#include "ECS/Component/RenderingComponent/ShadowMapComponents.hpp"
#include "ECS/Component/RenderingComponent/ShadowCasterRenderComponent.hpp"

#include "Renderer/GLObjects/SourceShader.hpp"
#include "Renderer/GLObjects/ResolvedShaderInstance.hpp"
#include "Renderer/GLObjects/Framebuffer.hpp"

#include "ResourceManager/ResourceManagers.hpp"

#include <glm/gtc/matrix_transform.hpp>

namespace neo {

	struct PointLightShadowMapParameters {
		float mNearPlane = 0.5f;
		std::optional<float> mFarPlaneOverride;
	};

	template<typename... CompTs>
	inline void drawPointLightShadows(RenderPasses& renderPasses, const ResourceManagers& resourceManagers, const ECS& ecs, const ECS::Entity& lightEntity, const bool clear, PointLightShadowMapParameters params = {}) {
		TRACY_ZONE();

		NEO_ASSERT(ecs.has<PointLightComponent>(lightEntity) && ecs.has<PointLightShadowMapComponent>(lightEntity), "Invalid light entity for point ligth shadows");
		TextureHandle shadowCubeHandle = ecs.cGetComponent<PointLightShadowMapComponent>(lightEntity)->mShadowMap;
		if (!resourceManagers.mTextureManager.isValid(shadowCubeHandle)) {
			return;
		}

		NEO_ASSERT(ecs.has<SpatialComponent>(lightEntity), "Point light shadows need a spatial");

		char targetName[128];
		for (int i = 0; i < 6; i++) {
			sprintf(targetName, "%s_%d_%d", "PointLightShadowMap", lightEntity, i);

			FramebufferHandle shadowTargetHandle = resourceManagers.mFramebufferManager.asyncLoad(
				HashedString(targetName),
				FramebufferExternalAttachments { 
					FramebufferAttachment {
						shadowCubeHandle,
						static_cast<types::framebuffer::AttachmentTarget>(static_cast<uint8_t>(types::framebuffer::AttachmentTarget::TargetCubeX_Positive) + i),
						0
					}
				},
				resourceManagers.mTextureManager
			);
			if (clear) {
				renderPasses.clear(shadowTargetHandle, types::framebuffer::AttachmentBit::Depth, glm::uvec4(0));
			}

			const Texture& shadowCube = resourceManagers.mTextureManager.resolve(shadowCubeHandle);
			renderPasses.renderPass(shadowTargetHandle, glm::uvec2(shadowCube.mWidth, shadowCube.mHeight), [i, lightEntity, params](const ResourceManagers& resourceManagers, const ECS& ecs) {
				TRACY_GPUN("Draw Face");
				auto shaderHandle = resourceManagers.mShaderManager.asyncLoad("PointLightShadowMap Shader", SourceShader::ConstructionArgs{
					{ types::shader::Stage::Vertex, "model.vert"},
					{ types::shader::Stage::Fragment, "pointlightdepth.frag" }
				});
				if (!resourceManagers.mShaderManager.isValid(shaderHandle)) {
					return;
				}
	
				static std::vector<std::vector<glm::vec3>> lookDirs = {
					{ glm::vec3( 1, 0, 0), glm::vec3( 0, -1, 0) },
					{ glm::vec3(-1, 0, 0), glm::vec3( 0, -1, 0) },
					{ glm::vec3( 0, 1, 0), glm::vec3( 0,  0, 1) },
					{ glm::vec3( 0,-1, 0), glm::vec3( 0,  0,-1) },
					{ glm::vec3( 0, 0, 1), glm::vec3( 0, -1, 0) },
					{ glm::vec3( 0, 0,-1), glm::vec3( 0, -1, 0) }
				};
				SpatialComponent cameraSpatial = *ecs.cGetComponent<SpatialComponent>(lightEntity); // Copy
				cameraSpatial.setLookDir(lookDirs[i][0], lookDirs[i][1]);
				FrustumComponent frustum;
				CameraComponent camera(
					params.mNearPlane, 
					params.mFarPlaneOverride.value_or(cameraSpatial.getScale().x / 2.f), 
					CameraComponent::Perspective{90.f, 1.f}
				);
				frustum.calculateFrustum(camera, cameraSpatial);
	
				ShaderDefines drawDefines;
				const auto& view = ecs.getView<const ShadowCasterRenderComponent, const MeshComponent, const SpatialComponent, CompTs...>();
				for (auto entity : view) {
					const SpatialComponent& drawSpatial = view.get<const SpatialComponent>(entity);
					// VFC
					if (ecs.has<BoundingBoxComponent>(entity) && !frustum.isInFrustum(drawSpatial, *ecs.cGetComponent<BoundingBoxComponent>(entity))) {
						continue;
					}
					drawDefines.reset();
	
					auto material = ecs.cGetComponent<const MaterialComponent>(entity);
	
					MakeDefine(ALPHA_TEST);
					bool containsAlphaTest = false;
					if constexpr ((std::is_same_v<AlphaTestComponent, CompTs> || ...) || (std::is_same_v<TransparentComponent, CompTs> || ...)) {
						containsAlphaTest = true;
					}
					bool doAlphaTest = containsAlphaTest && material && resourceManagers.mTextureManager.isValid(material->mAlbedoMap);
					if (doAlphaTest) {
						drawDefines.set(ALPHA_TEST);
					}
	
					auto& resolvedShader = resourceManagers.mShaderManager.resolveDefines(shaderHandle, drawDefines);
					resolvedShader.bind();
	
					if (doAlphaTest) {
						resolvedShader.bindTexture("alphaMap", resourceManagers.mTextureManager.resolve(material->mAlbedoMap));
					}
	
					resolvedShader.bindUniform("P", camera.getProj());
					resolvedShader.bindUniform("V", cameraSpatial.getView());
					resolvedShader.bindUniform("lightPos", cameraSpatial.getPosition());
					resolvedShader.bindUniform("lightRange", (cameraSpatial.getScale().x - 0.5) / 2.f);
	
					resolvedShader.bindUniform("M", drawSpatial.getModelMatrix());
					resourceManagers.mMeshManager.resolve(view.get<const MeshComponent>(entity).mMeshHandle).draw();
				}
			});
		}
	}
}
