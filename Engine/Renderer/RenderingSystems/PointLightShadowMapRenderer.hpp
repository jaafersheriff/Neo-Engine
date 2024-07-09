#pragma once

#include "ECS/ECS.hpp"

#include "ECS/Component/CameraComponent/CameraComponent.hpp"

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

		std::string handle = "pointLightShadow" + std::to_string(static_cast<int>(lightEntity));
		TextureHandle shadowCubeHandle = resourceManagers.mTextureManager.asyncLoad(
			HashedString(handle.c_str()),
			TextureBuilder{}
				.setDimension(glm::u16vec3(params.mDimension, params.mDimension, 0))
				.setFormat(TextureFormat{
					types::texture::Target::TextureCube,
					types::texture::InternalFormats::D16
				})
		);

		// TODO - this might break because cube
		FramebufferHandle shadowTargetHandle = resourceManagers.mFramebufferManager.asyncLoad(
			HashedString(handle.c_str()),
			FramebufferExternalHandles { shadowCubeHandle },
			resourceManagers.mTextureManager
		);
		if (!resourceManagers.mTextureManager.isValid(shadowCubeHandle) || !resourceManagers.mFramebufferManager.isValid(shadowTargetHandle)) {
			return NEO_INVALID_HANDLE;
		}

		auto& shadowTarget = resourceManagers.mFramebufferManager.resolve(shadowTargetHandle);
		shadowTarget.disableDraw();
		shadowTarget.bind();
		glViewport(0, 0, params.mDimension, params.mDimension);
		glCullFace(GL_FRONT);

		bool containsAlphaTest = false;
		if constexpr ((std::is_same_v<AlphaTestComponent, CompTs> || ...) || (std::is_same_v<TransparentComponent, CompTs> || ...)) {
			containsAlphaTest = true;
		}

		CameraComponent camera(1.f, 100.f, CameraComponent::Perspective{90.f, 1.f}); // TODO - parameters
		const glm::mat4 P = camera.getProj();
		const glm::vec3 lightPosition = ecs.cGetComponent<SpatialComponent>(lightEntity)->getPosition();
		static std::vector<std::vector<glm::vec3>> lookAndUpVectors = {
			{glm::vec3(1,0,0), glm::vec3(0,1,0)},
			{ glm::vec3(-1,0,0), glm::vec3(0,1,0) },
			{ glm::vec3(0,1,0), glm::vec3(0,0,1) },
			{ glm::vec3(0,-1,0), glm::vec3(0,0,-1) },
			{ glm::vec3(0,0,1), glm::vec3(0,1,0) },
			{ glm::vec3(0,0,-1), glm::vec3(0,1,0) }
		};
		for (int i = 0; i < 6; i++) {
			glm::mat4 V = glm::lookAt(lightPosition, lightPosition + lookAndUpVectors[i][0], lookAndUpVectors[i][1]);
			ShaderDefines drawDefines;
			const auto& view = ecs.getView<const ShadowCasterRenderComponent, const MeshComponent, const SpatialComponent, CompTs...>();
			for (auto entity : view) {
				// TODO - frustum culling is precomputed on camera components...
				// if (auto* culled = ecs.cGetComponent<CameraCulledComponent>(entity)) {
				// 	if (!culled->isInView(ecs, entity, shadowCameraEntity)) {
				// 		continue;
				// 	}
				// }
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

		glCullFace(GL_BACK);
		return shadowCubeHandle;
	}
}
