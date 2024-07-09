#pragma once

#include "ECS/ECS.hpp"

#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/CameraComponent/FrustumComponent.hpp"

#include "Renderer/GLObjects/SourceShader.hpp"
#include "Renderer/GLObjects/ResolvedShaderInstance.hpp"
#include "Renderer/GLObjects/Framebuffer.hpp"

#include "ResourceManager/ResourceManagers.hpp"

#include <glm/gtc/matrix_transform.hpp>

namespace neo {

	struct PointLightShadowParameters {
		uint32_t mDimension = 128;
	};

	template<typename... CompTs>
	inline TextureHandle drawPointLightShadows(const ResourceManagers& resourceManagers, const ECS& ecs, const ECS::Entity& lightEntity, const PointLightShadowParameters& params) {
		TRACY_GPU();
		auto shaderHandle = resourceManagers.mShaderManager.asyncLoad("ShadowMap Shader", SourceShader::ConstructionArgs{
			{ types::shader::Stage::Vertex, "model.vert"},
			{ types::shader::Stage::Fragment, "depth.frag" }
		});
		if (!resourceManagers.mShaderManager.isValid(shaderHandle)) {
			return NEO_INVALID_HANDLE;
		}

		std::string textureName = "pointLightShadow" + std::to_string(static_cast<int>(lightEntity));
		TextureHandle shadowCubeHandle = resourceManagers.mTextureManager.asyncLoad(
			HashedString(textureName.c_str()),
			TextureBuilder{}
				.setDimension(glm::u16vec3(params.mDimension, params.mDimension, 0))
				.setFormat(TextureFormat{
					types::texture::Target::TextureCube,
					types::texture::InternalFormats::D16
				})
		);

		glViewport(0, 0, params.mDimension, params.mDimension);
		glCullFace(GL_FRONT);

		bool containsAlphaTest = false;
		if constexpr ((std::is_same_v<AlphaTestComponent, CompTs> || ...) || (std::is_same_v<TransparentComponent, CompTs> || ...)) {
			containsAlphaTest = true;
		}

		FrustumComponent frustum;
		CameraComponent camera(1.f, 100.f, CameraComponent::Perspective{90.f, 1.f}); // TODO - parameters
		NEO_ASSERT(ecs.has<SpatialComponent>(lightEntity), "Point light shadows need a spatial");
		SpatialComponent cameraSpatial = *ecs.cGetComponent<SpatialComponent>(lightEntity); // Copy
		static std::vector<glm::vec3> lookDirs = {
			glm::vec3(1,0,0),
			glm::vec3(-1,0,0),
			glm::vec3(0,1,0),
			glm::vec3(0,-1,0),
			glm::vec3(0,0,1),
			glm::vec3(0,0,-1),
		};

		const auto& view = ecs.getView<const ShadowCasterRenderComponent, const MeshComponent, const SpatialComponent, CompTs...>();
		ShaderDefines drawDefines;
		std::string targetName = textureName + "_N";
		for (int i = 0; i < 6; i++) {
			TRACY_GPUN("Draw Face");
			cameraSpatial.setLookDir(lookDirs[i]);
			frustum.calculateFrustum(camera, cameraSpatial);

			types::framebuffer::AttachmentTarget target = static_cast<types::framebuffer::AttachmentTarget>(static_cast<uint8_t>(types::framebuffer::AttachmentTarget::TargetCubeX_Positive) + i);
			targetName.back() = '0' + static_cast<char>(i);
			FramebufferHandle shadowTargetHandle = resourceManagers.mFramebufferManager.asyncLoad(
				HashedString(targetName.c_str()),
				FramebufferExternalAttachments { 
					FramebufferAttachment {
						shadowCubeHandle,
						target,
						0
					}
				},
				resourceManagers.mTextureManager
			);
			if (!resourceManagers.mTextureManager.isValid(shadowCubeHandle) || !resourceManagers.mFramebufferManager.isValid(shadowTargetHandle)) {
				return NEO_INVALID_HANDLE;
			}
			auto& shadowTarget = resourceManagers.mFramebufferManager.resolve(shadowTargetHandle);
			shadowTarget.disableDraw();
			shadowTarget.bind();
			shadowTarget.clear(glm::uvec4(0.f, 0.f, 0.f, 0.f), types::framebuffer::AttachmentBit::Depth); // TODO - this will break if multiple passes (opaque, alphatest, etc) are called

			for (auto entity : view) {
				const SpatialComponent& drawSpatial = view.get<const SpatialComponent>(entity);
				// VFC
				if (ecs.has<BoundingBoxComponent>(entity) && !frustum.isInFrustum(drawSpatial, *ecs.cGetComponent<BoundingBoxComponent>(entity))) {
					continue;
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

				resolvedShader.bindUniform("P", camera.getProj());
				resolvedShader.bindUniform("V", cameraSpatial.getView());
				resolvedShader.bindUniform("M", drawSpatial.getModelMatrix());
				resourceManagers.mMeshManager.resolve(view.get<const MeshComponent>(entity).mMeshHandle).draw();
			}
		}

		glCullFace(GL_BACK);
		return shadowCubeHandle;
	}
}
