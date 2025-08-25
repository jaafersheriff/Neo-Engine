#pragma once

#include "ECS/ECS.hpp"

#include "ECS/Component/CameraComponent/CSMCameraComponent.hpp"
#include "ECS/Component/CameraComponent/FrustumComponent.hpp"

#include "Renderer/GLObjects/SourceShader.hpp"
#include "Renderer/GLObjects/ResolvedShaderInstance.hpp"
#include "Renderer/GLObjects/Framebuffer.hpp"

#include "Renderer/RenderingSystems/RenderPass.hpp"

#include "ResourceManager/ResourceManagers.hpp"

namespace neo {

	namespace {
		// This can be remapped to drawing a single shadow map tbh
		template<typename... CompTs>
		void _drawSingleCSM(
			RenderPasses& renderPasses,
			const ResourceManagers& resourceManagers,
			ECS::Entity cameraEntity,
			const TextureHandle& shadowMap,
			const ShaderHandle& shaderHandle,
			const int slice,
			const bool clear
		) {
			if (!resourceManagers.mTextureManager.isValid(shadowMap)) {
				return;
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

			if (clear) {
				renderPasses.clear(shadowMapHandle, types::framebuffer::AttachmentBit::Depth, glm::uvec4(0));
			}

			// Uhh is this important?
			// auto& shadowTarget = resourceManagers.mFramebufferManager.resolve(shadowMapHandle);
			// shadowTarget.disableDraw();
			// shadowTarget.disableRead();
			// shadowTarget.bind();
			const Texture& shadowTexture = resourceManagers.mTextureManager.resolve(shadowMap);
			renderPasses.declarePass(shadowMapHandle, glm::uvec2(shadowTexture.mWidth >> slice, shadowTexture.mHeight >> slice), [cameraEntity, shaderHandle](const ResourceManagers& resourceManagers, const ECS& ecs) {
				TRACY_GPUN("_drawSingleCSM");

				glCullFace(GL_FRONT);

				NEO_ASSERT(ecs.has<SpatialComponent>(cameraEntity) && ecs.has<CameraComponent>(cameraEntity), "Light entity is just wrong");
				const glm::mat4 P = ecs.cGetComponent<CameraComponent>(cameraEntity)->getProj();
				const glm::mat4 V = ecs.cGetComponent<SpatialComponent>(cameraEntity)->getView();

				ShaderDefines drawDefines;
				const auto view = ecs.getView<const ShadowCasterRenderComponent, const MeshComponent, const SpatialComponent, CompTs...>();
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

					resolvedShader.bindUniform("P", P);
					resolvedShader.bindUniform("V", V);
					resolvedShader.bindUniform("M", view.get<const SpatialComponent>(entity).getModelMatrix());
					resourceManagers.mMeshManager.resolve(view.get<const MeshComponent>(entity).mMeshHandle).draw();
				}

				glCullFace(GL_BACK);
			});
		}
	}

	struct CSMShadowInfo {
		bool mValidCSMShadows = false;
		std::array<glm::mat4, CSM_CAMERA_COUNT> mLightArrays = {};
	};
	inline CSMShadowInfo extractCSMShadowInfo(const ECS& ecs, const ECS::Entity lightEntity, const TextureManager& textureManager) {
		CSMShadowInfo ret;
		ret.mValidCSMShadows = true
			&& ecs.has<CameraComponent>(lightEntity)
			&& ecs.has<CSMShadowMapComponent>(lightEntity) && textureManager.isValid(ecs.cGetComponent<CSMShadowMapComponent>(lightEntity)->mShadowMap)
			&& ecs.has<CSMCamera0Component>()
			&& ecs.has<CSMCamera1Component>()
			&& ecs.has<CSMCamera2Component>();
		if (ret.mValidCSMShadows) {
			static glm::mat4 biasMatrix(
				0.5f, 0.0f, 0.0f, 0.0f,
				0.0f, 0.5f, 0.0f, 0.0f,
				0.0f, 0.0f, 0.5f, 0.0f,
				0.5f, 0.5f, 0.5f, 1.0f);
			auto csmCamera0Tuple = ecs.getSingleView<SpatialComponent, CameraComponent, CSMCamera0Component>();
			auto csmCamera1Tuple = ecs.getSingleView<SpatialComponent, CameraComponent, CSMCamera1Component>();
			auto csmCamera2Tuple = ecs.getSingleView<SpatialComponent, CameraComponent, CSMCamera2Component>();
			NEO_ASSERT(csmCamera0Tuple && csmCamera1Tuple && csmCamera2Tuple, "CSM Camera's dont exist");
			auto& [cameraEntity0, cameraSpatial0, csmCameraCamera0, csmCamera0] = *csmCamera0Tuple;
			auto& [cameraEntity1, cameraSpatial1, csmCameraCamera1, csmCamera1] = *csmCamera1Tuple;
			auto& [cameraEntity2, cameraSpatial2, csmCameraCamera2, csmCamera2] = *csmCamera2Tuple;

			ret.mLightArrays[0] = biasMatrix * csmCameraCamera0.getProj() * cameraSpatial0.getView();
			ret.mLightArrays[1] = biasMatrix * csmCameraCamera1.getProj() * cameraSpatial1.getView();
			ret.mLightArrays[2] = biasMatrix * csmCameraCamera2.getProj() * cameraSpatial2.getView();
		}

		return ret;
	}

	inline std::vector<ECS::EntityBuilder> createCSMCameras() {

		std::vector<ECS::EntityBuilder> ret;
		// CSM cameras
		// These need to be separate entities because 
		//	- CSM requires multiple Frustum and Camera components, and entities can only have one copy of a component
		//	- Frustum culling works off of camera entity
		// CSMFitting system is responsible for setting the various spatial/camera/frustum components
		auto csmCameraProto = ECS::EntityBuilder{}
			.attachComponent<SpatialComponent>()
			.attachComponent<CameraComponent>(-2.f, 2.f, CameraComponent::Orthographic{ glm::vec2(-4.f, 2.f), glm::vec2(0.1f, 5.f) })
			.attachComponent<FrustumComponent>()
			;
		ret.emplace_back(ECS::EntityBuilder(csmCameraProto)
			.attachComponent<CSMCamera0Component>()
		);
		ret.emplace_back(ECS::EntityBuilder(csmCameraProto)
			.attachComponent<CSMCamera1Component>()
		);
		ret.emplace_back(ECS::EntityBuilder(csmCameraProto)
			.attachComponent<CSMCamera2Component>()
		);

		return ret;
	}

	inline void removeCSMCameras(ECS& ecs) {
		if (auto csm0 = ecs.getSingleView<SpatialComponent, CSMCamera0Component>()) {
			ecs.removeEntity(std::get<0>(*csm0));
		}
		if (auto csm1 = ecs.getSingleView<SpatialComponent, CSMCamera1Component>()) {
			ecs.removeEntity(std::get<0>(*csm1));
		}
		if (auto csm2 = ecs.getSingleView<SpatialComponent, CSMCamera2Component>()) {
			ecs.removeEntity(std::get<0>(*csm2));
		}
	}

	template<typename... CompTs>
	inline void drawCSMShadows(RenderPasses& renderPasses, const ResourceManagers& resourceManagers, const ECS& ecs, ECS::Entity lightEntity, bool clear = false) {
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

		auto csmCamera0Tuple = ecs.getSingleView<SpatialComponent, CameraComponent, CSMCamera0Component>();
		auto csmCamera1Tuple = ecs.getSingleView<SpatialComponent, CameraComponent, CSMCamera1Component>();
		auto csmCamera2Tuple = ecs.getSingleView<SpatialComponent, CameraComponent, CSMCamera2Component>();
		NEO_ASSERT(csmCamera0Tuple && csmCamera1Tuple && csmCamera2Tuple, "CSM Camera's dont exist");
		auto& [cameraEntity0, cameraSpatial0, cameraCamera0, csmCamera0] = *csmCamera0Tuple;
		auto& [cameraEntity1, cameraSpatial1, cameraCamera1, csmCamera1] = *csmCamera1Tuple;
		auto& [cameraEntity2, cameraSpatial2, cameraCamera2, csmCamera2] = *csmCamera2Tuple;

		_drawSingleCSM<CompTs...>(renderPasses, resourceManagers, cameraEntity0, shadowMap->mShadowMap, shaderHandle, 0, clear);
		_drawSingleCSM<CompTs...>(renderPasses, resourceManagers, cameraEntity1, shadowMap->mShadowMap, shaderHandle, 1, clear);
		_drawSingleCSM<CompTs...>(renderPasses, resourceManagers, cameraEntity2, shadowMap->mShadowMap, shaderHandle, 2, clear);
	}
}
