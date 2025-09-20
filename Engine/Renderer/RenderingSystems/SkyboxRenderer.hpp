#pragma once

#include "Renderer/GLObjects/SourceShader.hpp"
#include "Renderer/GLObjects/ResolvedShaderInstance.hpp"

#include "ECS/ECS.hpp"
#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"
#include "ECS/Component/RenderingComponent/SkyboxComponent.hpp"

#include "Util/Util.hpp"

#include <tuple>

namespace neo {

	void drawSkybox(
		RenderPasses& renderPasses,
		const FramebufferHandle& outputTargetHandle,
		const glm::uvec2& viewport,
		ECS::Entity cameraEntity
	) {
		TRACY_ZONE();

		RenderState renderState;
		renderState.mCullFace = std::nullopt;
		renderState.mDepthState = DepthState{
			DepthFunc::LessEqual,
			false
		};

		renderPasses.renderPass(outputTargetHandle, viewport, renderState, [cameraEntity](const ResourceManagers& resourceManagers, const ECS& ecs) {
			TRACY_GPU();

			auto skyboxTuple = ecs.cGetComponent<SkyboxComponent>();
			auto skyboxShaderHandle = resourceManagers.mShaderManager.asyncLoad("SkyboxShader", SourceShader::ConstructionArgs{
				{ types::shader::Stage::Vertex, "skybox.vert"},
				{ types::shader::Stage::Fragment, "skybox.frag" }
				});
			if (!skyboxTuple || !resourceManagers.mShaderManager.isValid(skyboxShaderHandle)) {
				return;
			}
			const auto& [_, skybox] = *skyboxTuple;
			if (!resourceManagers.mTextureManager.isValid(skybox.mSkybox)) {
				return;
			}
			const auto& skyboxTexture = resourceManagers.mTextureManager.resolve(skybox.mSkybox);

			auto camera = ecs.cGetComponent<CameraComponent>(cameraEntity);
			auto camSpatial = ecs.cGetComponent<SpatialComponent>(cameraEntity);
			NEO_ASSERT(camera, "No main camera exists");

			ShaderDefines defines;
			MakeDefine(EQUIRECTANGULAR);
			MakeDefine(HDR);
			if (skyboxTexture.mFormat.mTarget == types::texture::Target::Texture2D) {
				defines.set(EQUIRECTANGULAR);
			}
			if (skyboxTexture.mFormat.mType != types::ByteFormats::UnsignedByte) {
				defines.set(HDR);
			}

			auto& resolvedShader = resourceManagers.mShaderManager.resolveDefines(skyboxShaderHandle, defines);

			resolvedShader.bind();
			resolvedShader.bindUniform("P", camera->getProj());
			resolvedShader.bindUniform("V", camSpatial->getView());
			resolvedShader.bindTexture("cubeMap", skyboxTexture);

			/* Draw */
			resourceManagers.mMeshManager.resolve(HashedString("cube")).draw();
		}, "Draw Skybox");
	}
}