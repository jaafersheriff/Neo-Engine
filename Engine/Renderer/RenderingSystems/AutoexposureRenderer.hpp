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

	void calculateAutoexposure(const ResourceManagers& resourceManagers, TextureHandle previousFrameHDRColor, float minLuminance = 0.f, float maxLuminance = 10.f) {
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
			{
				MakeDefine(CLEAR);
				ShaderDefines defines;
				defines.set(CLEAR);

				auto& clearShader = resourceManagers.mShaderManager.resolveDefines(histogramShaderHandle, defines);
				auto barrier = clearShader.bindShaderBuffer("histogramBuffer", histogram.glId, types::shader::Access::Write);
				clearShader.dispatch({ 256 / 16, 256 / 16, 1 });
			}

			{
				MakeDefine(POPULATE);
				ShaderDefines defines;
				defines.set(POPULATE);

				auto& populateShader = resourceManagers.mShaderManager.resolveDefines(histogramShaderHandle, defines);
				const auto& inputTexture = resourceManagers.mTextureManager.resolve(previousFrameHDRColor);

				populateShader.bindUniform("inputResolution", glm::uvec2(inputTexture.mWidth, inputTexture.mHeight));
				populateShader.bindUniform("minLogLum", glm::log2(minLuminance + util::EP));
				populateShader.bindUniform("inverseLogLumRange", 1.f / glm::log2(maxLuminance - minLuminance) + util::EP);
				auto imageBarrier = populateShader.bindImageTexture("inputTexture", inputTexture, types::shader::Access::Read);
				auto histogramBarrier = populateShader.bindShaderBuffer("histogramBuffer", histogram.glId, types::shader::Access::Write);
				populateShader.dispatch({ inputTexture.mWidth / 16, inputTexture.mHeight / 16, 1 });
			}
			{
				glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
				glBindBuffer(GL_SHADER_STORAGE_BUFFER, histogram.glId);
				unsigned int data[256];
				glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(unsigned int) * 256, data);
				if (data) {
					std::stringstream ss;
					for (int i = 0; i < 256; i++) {
						ss << data[i] << ", ";
					}
					NEO_LOG_V("%s", ss.str().c_str());
				}
			}
		}
	}
}