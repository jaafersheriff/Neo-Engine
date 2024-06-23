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

	void drawSkybox(const ResourceManagers& resourceManagers, const ECS& ecs, ECS::Entity cameraEntity) {
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

		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

		auto camera = ecs.cGetComponent<CameraComponent>(cameraEntity);
		auto camSpatial = ecs.cGetComponent<SpatialComponent>(cameraEntity);
		NEO_ASSERT(camera, "No main camera exists");

		ShaderDefines defines;
		MakeDefine(EQUIRECTANGULAR);
		if (skybox.mEquirectangular) {
			defines.set(EQUIRECTANGULAR);
		}
		auto& resolvedShader = resourceManagers.mShaderManager.resolveDefines(skyboxShaderHandle, defines);

		resolvedShader.bind();
		resolvedShader.bindUniform("P", camera->getProj());
		resolvedShader.bindUniform("V", camSpatial->getView());
		resolvedShader.bindTexture("cubeMap", resourceManagers.mTextureManager.resolve(skybox.mSkybox));

		/* Draw */
		resourceManagers.mMeshManager.resolve(HashedString("cube")).draw();

		glEnable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);
	}
}