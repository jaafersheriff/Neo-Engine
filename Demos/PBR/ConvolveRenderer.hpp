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
			ibl.mDFGLut = resourceManagers.mTextureManager.asyncLoad(
				HashedString("dfgLut"),
				TextureBuilder{}
				.setDimension(glm::u16vec3(ibl.mDFGLutResolution, ibl.mDFGLutResolution, 0))
				.setFormat(TextureFormat{
					types::texture::Target::Texture2D,
					types::texture::InternalFormats::RGBA8_UNORM,
					TextureFilter { types::texture::Filters::Linear, types::texture::Filters::Linear },
					TextureWrap { types::texture::Wraps::Clamp, types::texture::Wraps::Clamp, types::texture::Wraps::Clamp },
					types::ByteFormats::Float
					})
			);
		}
		if (resourceManagers.mTextureManager.isValid(ibl.mDFGLut) && !ibl.mDFGGenerated) {
			auto& dfgLutShader = resourceManagers.mShaderManager.resolveDefines(dfgLutShaderHandle, {});
			dfgLutShader.bind();

			dfgLutShader.bindTexture("dst", resourceManagers.mTextureManager.resolve(ibl.mDFGLut));
			glBindImageTexture(0, resourceManagers.mTextureManager.resolve(ibl.mDFGLut).mTextureID, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F/*resourceManagers.mTextureManager.resolve(ibl.mDFGLut).mFormat.mInternalFormat*/);
			glDispatchCompute(
				ibl.mDFGLutResolution / 8,
				ibl.mDFGLutResolution / 8,
				1
			);
			glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

			ibl.mDFGGenerated = true;
		}
	}
}