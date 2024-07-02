#pragma once

#include "Renderer/Renderer.hpp"
#include "Renderer/GLObjects/SourceShader.hpp"
#include "Renderer/GLObjects/ResolvedShaderInstance.hpp"

#include "ECS/ECS.hpp"

#include "ResourceManager/ResourceManagers.hpp"

#include "Util/Util.hpp"

#include <tuple>

namespace neo {
	namespace {

		// TODO - wtf do I do with this...
		struct ShaderBuffer {
			uint32_t glId = 0;
			bool isInit = false;

			void init(uint32_t byteSize) {
				NEO_UNUSED(byteSize);
				glGenBuffers(1, &glId);
				glBindBuffer(GL_SHADER_STORAGE_BUFFER, glId);
				glBufferData(GL_SHADER_STORAGE_BUFFER, byteSize, 0, GL_DYNAMIC_DRAW);
				isInit = true;

			}
			void destroy() {
				glDeleteBuffers(1, &glId);
			}
		};
	}

	void calculateAutoexposure(const ResourceManagers& resourceManagers, TextureHandle previousFrameHDRColor) {
		TRACY_GPU();

		if (!resourceManagers.mTextureManager.isValid(previousFrameHDRColor)) {
			return;
		}

		static ShaderBuffer histogram;
		if (!histogram.isInit) {
			histogram.init(sizeof(unsigned int) * 256);
		}

		auto histogramShaderHandle = resourceManagers.mShaderManager.asyncLoad("Histogram Shader", SourceShader::ConstructionArgs{
			{types::shader::Stage::Compute, "histogram.comp" }
		});
		if (resourceManagers.mShaderManager.isValid(histogramShaderHandle)) {
			MakeDefine(CLEAR);
			ShaderDefines defines;
			defines.set(CLEAR);

			{
				auto& clearShader = resourceManagers.mShaderManager.resolveDefines(histogramShaderHandle, defines);
				auto barrier = clearShader.bindShaderBuffer("histogramBuffer", histogram.glId, types::shader::Access::Write);
				clearShader.dispatch({ 256 / 16, 256/16, 1 });
			}

			{
				auto& populateShader = resourceManagers.mShaderManager.resolveDefines(histogramShaderHandle, {});
				const auto& inputTexture = resourceManagers.mTextureManager.resolve(previousFrameHDRColor);

				auto imageBarrier = populateShader.bindImageTexture("inputTexture", inputTexture, types::shader::Access::Read);
				auto histogramBarrier = populateShader.bindShaderBuffer("histogramBuffer", histogram.glId, types::shader::Access::Write);
				populateShader.dispatch({ inputTexture.mWidth / 16, inputTexture.mHeight / 16, 1 });
			}
		}
	}
}