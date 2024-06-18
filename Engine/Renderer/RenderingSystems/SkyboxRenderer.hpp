#pragma once

#include "Renderer/GLObjects/SourceShader.hpp"
#include "Renderer/GLObjects/ResolvedShaderInstance.hpp"

#include "ECS/ECS.hpp"
#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/CameraComponent/PerspectiveCameraComponent.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"
#include "ECS/Component/RenderingComponent/SkyboxComponent.hpp"

#include "Util/Util.hpp"

#include <tuple>

namespace neo {

	void drawSkybox(const ResourceManagers& resourceManagers, const ECS& ecs, ECS::Entity cameraEntity) {
		TRACY_GPU();

		auto skybox = ecs.cGetComponent<SkyboxComponent>();
		auto skyboxShaderHandle = resourceManagers.mShaderManager.asyncLoad("SkyboxShader", SourceShader::ShaderCode{
			{ types::shader::Stage::Vertex, R"(
				layout (location = 0) in vec3 vertPos;
				uniform mat4 P;
				uniform mat4 V;
				out vec3 fragTex;
				void main() {
					mat4 skyV = V;
					skyV[3][0] = skyV[3][1] = skyV[3][2] = 0.0;
					vec4 pos = P * skyV * vec4(vertPos, 1.0); 
					gl_Position = pos.xyww;
					fragTex = vertPos;
			})"},
			{ types::shader::Stage::Fragment, R"(
				in vec3 fragTex;
				layout(binding = 0) uniform samplerCube cubeMap;
				out vec4 color;
				void main() {
					color = texture(cubeMap, fragTex);
			})" }
		});
		if (!skybox || !resourceManagers.mShaderManager.isValid(skyboxShaderHandle)) {
			return;
		}

		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

		auto camera = ecs.cGetComponentAs<CameraComponent, PerspectiveCameraComponent>(cameraEntity);
		auto camSpatial = ecs.cGetComponent<SpatialComponent>(cameraEntity);
		NEO_ASSERT(camera, "No main camera exists");

		auto& resolvedShader = resourceManagers.mShaderManager.resolveDefines(skyboxShaderHandle, {});
		resolvedShader.bind();
		resolvedShader.bindUniform("P", camera->getProj());
		resolvedShader.bindUniform("V", camSpatial->getView());
		resolvedShader.bindTexture("cubeMap", resourceManagers.mTextureManager.resolve(std::get<1>(*skybox).mSkybox));

		/* Draw */
		resourceManagers.mMeshManager.resolve(HashedString("cube")).draw();

		glEnable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);
	}
}