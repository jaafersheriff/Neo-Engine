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
		FrameGraph& fg,
		FramebufferHandle outTarget,
		Viewport vp,
		const ResourceManagers& resourceManagers,
		ECS::Entity cameraEntity) {
		TRACY_ZONE();

		auto skyboxShaderHandle = resourceManagers.mShaderManager.asyncLoad("SkyboxShader", SourceShader::ConstructionArgs{
			{ types::shader::Stage::Vertex, "skybox.vert"},
			{ types::shader::Stage::Fragment, "skybox.frag" }
			});


		PassState passState;
		passState.mCullFace = false;
		passState.mDepthMask = false;
		passState.mDepthFunc = types::passState::DepthFunc::LessEqual;
		fg.pass(outTarget, vp, vp, passState, skyboxShaderHandle)
			.with([cameraEntity](Pass& pass, const ResourceManagers& resourceManagers, const ECS& ecs) {
			auto skyboxTuple = ecs.cGetComponent<SkyboxComponent>();
			if (!skyboxTuple) {
				return;
			}
			const auto& [_, skybox] = *skyboxTuple;


			auto camera = ecs.cGetComponent<CameraComponent>(cameraEntity);
			auto camSpatial = ecs.cGetComponent<SpatialComponent>(cameraEntity);
			NEO_ASSERT(camera, "No main camera exists");
			const auto& skyboxTexture = resourceManagers.mTextureManager.resolve(skybox.mSkybox);
			MakeDefine(EQUIRECTANGULAR);
			MakeDefine(HDR);
			if (skyboxTexture.mFormat.mTarget == types::texture::Target::Texture2D) {
				pass.setDefine(EQUIRECTANGULAR);
			}
			if (skyboxTexture.mFormat.mType != types::ByteFormats::UnsignedByte) {
				pass.setDefine(HDR);
			}

			pass.bindUniform("P", camera->getProj());
			pass.bindUniform("V", camSpatial->getView());
			pass.bindTexture("cubeMap", skybox.mSkybox);

			pass.drawCommand(HashedString("cube"), {}, {});

			})
			.setDebugName("Skybox");
	}
}