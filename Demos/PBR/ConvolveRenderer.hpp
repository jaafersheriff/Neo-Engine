#pragma once

#include "Renderer/GLObjects/SourceShader.hpp"
#include "Renderer/GLObjects/ResolvedShaderInstance.hpp"

#include "ECS/ECS.hpp"
#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/CameraComponent/PerspectiveCameraComponent.hpp"
#include "ECS/Component/RenderingComponent/SkyboxComponent.hpp"
#include "PBR/IBLComponent.hpp"

#include "ResourceManager/ResourceManagers.hpp"

#include "Util/Util.hpp"

#include <tuple>

namespace PBR {

	void convolveCubemap(const ResourceManagers& resourceManagers, const ECS& ecs) {
		TRACY_GPU();

		auto skybox = ecs.getSingleView<SkyboxComponent, IBLComponent>();
		auto convolveShader = resourceManagers.mShaderManager.asyncLoad("ConvolveShader", SourceShader::ConstructionArgs{...});
		auto dfgLutShader = resourceManagers.mShaderManager.asyncLoad("DFGLutShader", SourceShader::ConstructionArgs{...});
		if (!skybox || !resourceManagers.mShaderManager.isValid(convolveShader) || !resourceManagers.mShaderManager.isValid(dfgLutShader)) {
			return;
		}

		// TODO
	}
}