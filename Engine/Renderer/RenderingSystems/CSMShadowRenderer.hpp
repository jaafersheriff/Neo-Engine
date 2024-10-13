#pragma once

#include "ECS/ECS.hpp"

#include "ECS/Component/CameraComponent/CSMCameraComponent.hpp"

#include "Renderer/GLObjects/SourceShader.hpp"
#include "Renderer/GLObjects/ResolvedShaderInstance.hpp"
#include "Renderer/GLObjects/Framebuffer.hpp"

#include "ResourceManager/ResourceManagers.hpp"

namespace neo {

	namespace {
		template<typename... CompTs>
		void _drawSingleCSM(
			const ResourceManagers& resourceManagers, 
			const ECS& ecs, 
			ECS::Entity cameraEntity, 
			const TextureHandle& shadowMap, 
			const ShaderHandle& shaderHandle, 
			const int slice, 
			const bool clear
		) {
			TRACY_GPU();
			bool containsAlphaTest = false;
			if constexpr ((std::is_same_v<AlphaTestComponent, CompTs> || ...) || (std::is_same_v<TransparentComponent, CompTs> || ...)) {
				containsAlphaTest = true;
			}

			char targetName[32];
			sprintf(targetName, "ShadowMap_%d_%d", static_cast<int>(cameraEntity), slice);
			FramebufferHandle shadowMapHandle = resourceManagers.mFramebufferManager.asyncLoad(
				HashedString(targetName),
				FramebufferExternalAttachments{ {
						shadowMap,
						types::framebuffer::AttachmentTarget::Target2D,
						static_cast<uint8_t>(slice)
				} },
				resourceManagers.mTextureManager
			);
			if (!resourceManagers.mFramebufferManager.isValid(shadowMapHandle)) {
				return;
			}
			auto& shadowTarget = resourceManagers.mFramebufferManager.resolve(shadowMapHandle);
			shadowTarget.disableDraw();
			shadowTarget.disableRead();
			shadowTarget.bind();
			{
				const auto& shadowTexture = resourceManagers.mTextureManager.resolve(shadowMap);
				glViewport(0, 0, shadowTexture.mWidth >> slice, shadowTexture.mHeight >> slice);
			}

			if (clear) {
				shadowTarget.clear(glm::uvec4(0.f, 0.f, 0.f, 0.f), types::framebuffer::AttachmentBit::Depth);
			}

			NEO_ASSERT(ecs.has<SpatialComponent>(cameraEntity) && ecs.has<CameraComponent>(cameraEntity), "Light entity is just wrong");
			const glm::mat4 P = ecs.cGetComponent<CameraComponent>(cameraEntity)->getProj();
			const glm::mat4 V = ecs.cGetComponent<SpatialComponent>(cameraEntity)->getView();

			ShaderDefines drawDefines;
			const auto& view = ecs.getView<const ShadowCasterRenderComponent, const MeshComponent, const SpatialComponent, CompTs...>();
			for (auto entity : view) {
				// VFC
				if (auto* culled = ecs.cGetComponent<CameraCulledComponent>(entity)) {
					if (!culled->isInView(ecs, entity, cameraEntity)) {
						continue;
					}
				}
				drawDefines.reset();

				auto material = ecs.cGetComponent<const MaterialComponent>(entity);

				MakeDefine(ALPHA_TEST);
				bool doAlphaTest = containsAlphaTest && material && resourceManagers.mTextureManager.isValid(material->mAlbedoMap);
				if (doAlphaTest) {
					drawDefines.set(ALPHA_TEST);
				}

				auto& resolvedShader = resourceManagers.mShaderManager.resolveDefines(shaderHandle, drawDefines);
				resolvedShader.bind();

				if (doAlphaTest) {
					resolvedShader.bindTexture("alphaMap", resourceManagers.mTextureManager.resolve(material->mAlbedoMap));
				}

				resolvedShader.bindUniform("P", P);
				resolvedShader.bindUniform("V", V);
				resolvedShader.bindUniform("M", view.get<const SpatialComponent>(entity).getModelMatrix());
				resourceManagers.mMeshManager.resolve(view.get<const MeshComponent>(entity).mMeshHandle).draw();
			}
		}
	}

	template<typename... CompTs>
	inline void drawCSMShadows(const ResourceManagers& resourceManagers, const ECS& ecs, ECS::Entity lightEntity, bool clear) {
		TRACY_GPU();
		auto shaderHandle = resourceManagers.mShaderManager.asyncLoad("ShadowMap Shader", SourceShader::ConstructionArgs{
			{ types::shader::Stage::Vertex, "model.vert"},
			{ types::shader::Stage::Fragment, "depth.frag" }
		});
		if (!resourceManagers.mShaderManager.isValid(shaderHandle)) {
			return;
		}

		NEO_ASSERT(ecs.has<DirectionalLightComponent>(lightEntity) && ecs.has<CSMShadowMapComponent>(lightEntity), "Invalid light entity");
		auto shadowMap = ecs.cGetComponent<CSMShadowMapComponent>(lightEntity);
		if (!resourceManagers.mTextureManager.isValid(shadowMap->mShadowMap)) {
			return;
		}

		glCullFace(GL_FRONT);

		// TODO - this should have asserts
		if (auto csmCamera0 = ecs.getSingleView<SpatialComponent, CameraComponent, CSMCamera0Component>()) {
			auto& [cameraEntity, csmSpatial, csmCamera, csm] = *csmCamera0;
			_drawSingleCSM<CompTs...>(resourceManagers, ecs, cameraEntity, shadowMap->mShadowMap, shaderHandle, csm.getLod(), clear);
		}
		if (auto csmCamera1 = ecs.getSingleView<SpatialComponent, CameraComponent, CSMCamera1Component>()) {
			auto& [cameraEntity, csmSpatial, csmCamera, csm] = *csmCamera1;
			_drawSingleCSM<CompTs...>(resourceManagers, ecs, cameraEntity, shadowMap->mShadowMap, shaderHandle, csm.getLod(), clear);
		}
		if (auto csmCamera2 = ecs.getSingleView<SpatialComponent, CameraComponent, CSMCamera2Component>()) {
			auto& [cameraEntity, csmSpatial, csmCamera, csm] = *csmCamera2;
			_drawSingleCSM<CompTs...>(resourceManagers, ecs, cameraEntity, shadowMap->mShadowMap, shaderHandle, csm.getLod(), clear);
		}
		if (auto csmCamera3 = ecs.getSingleView<SpatialComponent, CameraComponent, CSMCamera3Component>()) {
			auto& [cameraEntity, csmSpatial, csmCamera, csm] = *csmCamera3;
			_drawSingleCSM<CompTs...>(resourceManagers, ecs, cameraEntity, shadowMap->mShadowMap, shaderHandle, csm.getLod(), clear);
		}
		glCullFace(GL_BACK);
	}
}
