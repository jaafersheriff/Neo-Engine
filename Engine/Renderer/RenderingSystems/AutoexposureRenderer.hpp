#pragma once

#include "Renderer/Renderer.hpp"
#include "Renderer/GLObjects/SourceShader.hpp"
#include "Renderer/GLObjects/ResolvedShaderInstance.hpp"

#include "ECS/ECS.hpp"

#include "ResourceManager/ResourceManagers.hpp"

#include "Util/Util.hpp"

#include <tuple>

namespace neo {

	struct AutoExposureParameters {
		float mMinLuminance = 0.f;
		float mMaxLuminance = 10.f;
		float mTimeCoefficient = 1.f;

		void imguiEditor() {
			ImGui::SliderFloat("Min Lum", &mMinLuminance, 0.001f, mMaxLuminance);
			ImGui::SliderFloat("Max Lum", &mMaxLuminance, mMinLuminance, 100.f);
			ImGui::SliderFloat("Time Coeff", &mTimeCoefficient, 0.001f, 1.f);
		}
	};

	inline TextureHandle calculateAutoexposure(const ResourceManagers& resourceManagers, const TextureHandle previousFrameHDR, const AutoExposureParameters& params) {
		TRACY_GPU();

		if (!resourceManagers.mTextureManager.isValid(previousFrameHDR)) {
			return NEO_INVALID_HANDLE;
		}
		const auto& previousFrame = resourceManagers.mTextureManager.resolve(previousFrameHDR);

		auto histogram = resourceManagers.mTextureManager.asyncLoad("Histogram", TextureBuilder{}
			.setDimension({ 256, 1, 0 })
			.setFormat(TextureFormat{ 
				types::texture::Target::Texture2D, 
				types::texture::InternalFormats::R32_UI, 
				TextureFilter {types::texture::Filters::Nearest, types::texture::Filters::Nearest}, // R32UI needs nearest filter wahoo
				TextureWrap{ types::texture::Wraps::Mirrored, types::texture::Wraps::Mirrored}
			})
		);
		if (!resourceManagers.mTextureManager.isValid(histogram)) {
			return NEO_INVALID_HANDLE;
		}

		auto histogramPopulateHandle = resourceManagers.mShaderManager.asyncLoad("HistogramPopulate Shader", SourceShader::ConstructionArgs{
			{types::shader::Stage::Compute, "histogram_populate.comp" }
			});
		if (resourceManagers.mShaderManager.isValid(histogramPopulateHandle)) {
			auto& populateShader = resourceManagers.mShaderManager.resolveDefines(histogramPopulateHandle, {});

			populateShader.bindUniform("inputResolution", glm::uvec2(previousFrame.mWidth, previousFrame.mHeight));
			populateShader.bindUniform("minLogLum", glm::log2(params.mMinLuminance + util::EP));
			populateShader.bindUniform("inverseLogLumRange", 1.f / (glm::log2(glm::abs(params.mMaxLuminance - params.mMinLuminance + util::EP))));
			auto imageBarrier1 = populateShader.bindImageTexture("previousHDRColor", previousFrame, types::shader::Access::Read);
			auto imageBarrier2 = populateShader.bindImageTexture("histogram", resourceManagers.mTextureManager.resolve(histogram), types::shader::Access::ReadWrite);
			populateShader.dispatch({ previousFrame.mWidth / 16, previousFrame.mHeight / 16, 1 });
		}

		auto outputTexture = resourceManagers.mTextureManager.asyncLoad("Histogram Average", TextureBuilder{}
			.setDimension({ 1, 1, 0 })
			.setFormat(TextureFormat{
				types::texture::Target::Texture2D,
				types::texture::InternalFormats::R16_F,
				TextureFilter { types::texture::Filters::Nearest, types::texture::Filters::Nearest } 
			})
		);
		// auto histogramAverageHandle = resourceManagers.mShaderManager.asyncLoad("HistogramAverage Shader", SourceShader::ConstructionArgs{
		// 	{types::shader::Stage::Compute, "histogram_average.comp" }
		// });
		// if (resourceManagers.mTextureManager.isValid(outputTexture) && resourceManagers.mShaderManager.isValid(histogramAverageHandle)) {
		// 	auto& averageShader = resourceManagers.mShaderManager.resolveDefines(histogramAverageHandle, {});
		// 	averageShader.bindUniform("inputResolution", glm::uvec2(previousFrame.mWidth, previousFrame.mHeight));
		// 	averageShader.bindUniform("minLogLum", glm::log2(params.mMinLuminance + util::EP));
		// 	averageShader.bindUniform("logLumRange", glm::log2(glm::abs(params.mMaxLuminance - params.mMinLuminance + util::EP)));
		// 	averageShader.bindUniform("timeCoefficient", params.mTimeCoefficient);
		// 	auto histogramBarrier = averageShader.bindShaderBuffer("histogramBuffer", histogram.glId, types::shader::Access::ReadWrite);
		// 	auto imageBarrier = averageShader.bindImageTexture("dst", resourceManagers.mTextureManager.resolve(outputTexture), types::shader::Access::Write);
		// 	averageShader.dispatch({ 1, 1, 1 });

		// }
		return outputTexture;
	}
}