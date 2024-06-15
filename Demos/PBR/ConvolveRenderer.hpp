#pragma once

#include "Renderer/Renderer.hpp"
#include "Renderer/GLObjects/SourceShader.hpp"
#include "Renderer/GLObjects/ResolvedShaderInstance.hpp"

#include "ECS/ECS.hpp"
#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/CameraComponent/PerspectiveCameraComponent.hpp"
#include "ECS/Component/RenderingComponent/SkyboxComponent.hpp"
#include "PBR/IBLComponent.hpp"

#include "ResourceManager/ResourceManagers.hpp"

#include "Util/Util.hpp"
#include "Util/ServiceLocator.hpp"

#include <tuple>

namespace PBR {

	void convolveCubemap(const ResourceManagers& resourceManagers, const ECS& ecs) {
		TRACY_GPU();

		auto skyboxTuple = ecs.getSingleView<SkyboxComponent, IBLComponent>();
		auto convolveShaderHandle = resourceManagers.mShaderManager.asyncLoad("ConvolveShader", SourceShader::ConstructionArgs { 
			{ types::shader::Stage::Compute, "pbr/convolve.comp" }
		});
		auto dfgLutShaderHandle = resourceManagers.mShaderManager.asyncLoad("DFGLutShader", SourceShader::ConstructionArgs{
			{ types::shader::Stage::Compute, "pbr/dfglut.comp" }
		});
		if (!skyboxTuple || !resourceManagers.mShaderManager.isValid(convolveShaderHandle) || !resourceManagers.mShaderManager.isValid(dfgLutShaderHandle)) {
			return;
		}

		const SkyboxComponent& skybox = std::get<1>(*skyboxTuple);
		const IBLComponent& ibl = std::get<2>(*skyboxTuple);

		if (!resourceManagers.mTextureManager.isValid(skybox.mSkybox)) {
			NEO_LOG_W("There's no skybox texture to convolve");
			return;
		}
		const Texture& skyboxCubemap = resourceManagers.mTextureManager.resolve(skybox.mSkybox);
		if (!skyboxCubemap.mFormat.mFilter.usesMipFilter()) {
			NEO_LOG_E("Skybox cubemap needs to have mips");
			return;
		}

		if (ibl.mDFGLut == NEO_INVALID_HANDLE) {
			auto& dfgLutShader = resourceManagers.mShaderManager.resolveDefines(dfgLutShaderHandle, {});
			dfgLutShader.bind();

			glDispatchCompute(
				ibl.mDFGLutResolution / ServiceLocator<Renderer>::ref().mDetails.mMaxComputeWorkGroupSize.x,
				ibl.mDFGLutResolution / ServiceLocator<Renderer>::ref().mDetails.mMaxComputeWorkGroupSize.y, 
				1
			);
		}
	}
}