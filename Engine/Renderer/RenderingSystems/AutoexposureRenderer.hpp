#pragma once

#include "Renderer/Renderer.hpp"
#include "Renderer/GLObjects/SourceShader.hpp"
#include "Renderer/GLObjects/ResolvedShaderInstance.hpp"
#include "Renderer/RenderingSystems/RenderPass.hpp"

#include "ECS/ECS.hpp"
#include "ECS/Component/EngineComponents/FrameStatsComponent.hpp"

#include "ResourceManager/ResourceManagers.hpp"

#include "Util/Util.hpp"

#include <tuple>

namespace neo {

	struct AutoExposureParameters {
		float mMinLogLuminance = util::EP;
		float mMaxLogLuminance = 10.f;
		float mTimeCoefficient = 1.f + util::EP;

		void imguiEditor() {
			if (ImGui::TreeNodeEx("AutoExposure Parameters", ImGuiTreeNodeFlags_DefaultOpen)) {
				ImGui::SliderFloat("Min Log Lum", &mMinLogLuminance, util::EP, mMaxLogLuminance);
				ImGui::SliderFloat("Max Log Lum", &mMaxLogLuminance, mMinLogLuminance, 100.f);
				ImGui::SliderFloat("Time Coeff", &mTimeCoefficient, 0.001f, 1.f);

				ImGui::TreePop();
			}
		}
	};

	inline TextureHandle calculateAutoexposure(RenderPasses& renderPasses, const ResourceManagers& resourceManagers, const ECS&, const TextureHandle previousFrameHDR, AutoExposureParameters& params) {
		TRACY_ZONE();

		if (!resourceManagers.mTextureManager.isValid(previousFrameHDR)) {
			return NEO_INVALID_HANDLE;
		}

		auto histogramHandle = resourceManagers.mTextureManager.asyncLoad("Histogram", TextureBuilder{}
			.setDimension({ 256, 1, 0 })
			.setFormat(TextureFormat{ 
				types::texture::Target::Texture2D, 
				types::texture::InternalFormats::R32_UI, 
				TextureFilter {types::texture::Filters::Nearest, types::texture::Filters::Nearest}, // R32UI needs nearest filter wahoo
				TextureWrap{ types::texture::Wraps::Mirrored, types::texture::Wraps::Mirrored}
			})
		);
		if (!resourceManagers.mTextureManager.isValid(histogramHandle)) {
			return NEO_INVALID_HANDLE;
		}

		renderPasses.computePass([histogramHandle](const ResourceManagers& resourceManagers, const ECS&) {
			TRACY_GPUN("Histogram clear");
			auto histogramClearHandle = resourceManagers.mShaderManager.asyncLoad("HistogramClear Shader", SourceShader::ConstructionArgs{
				{types::shader::Stage::Compute, "histogram_clear.comp" }
			});
			if (resourceManagers.mShaderManager.isValid(histogramClearHandle)) {
				auto& clearShader = resourceManagers.mShaderManager.resolveDefines(histogramClearHandle, {});
				auto imageBarrier2 = clearShader.bindImageTexture("histogram", resourceManagers.mTextureManager.resolve(histogramHandle), types::shader::Access::Write);
				clearShader.dispatch({ 16, 16, 1 });
			}
		}, "Histogram clear");

		renderPasses.computePass([histogramHandle, previousFrameHDR, params](const ResourceManagers& resourceManagers, const ECS&) {
			TRACY_GPUN("Histogram populate");
			if (!resourceManagers.mTextureManager.isValid(previousFrameHDR)) {
				return;
			}
			auto histogramPopulateHandle = resourceManagers.mShaderManager.asyncLoad("HistogramPopulate Shader", SourceShader::ConstructionArgs{
				{types::shader::Stage::Compute, "histogram_populate.comp" }
				});
			if (resourceManagers.mShaderManager.isValid(histogramPopulateHandle)) {
				auto& populateShader = resourceManagers.mShaderManager.resolveDefines(histogramPopulateHandle, {});

				const auto& previousFrame = resourceManagers.mTextureManager.resolve(previousFrameHDR);
				populateShader.bindUniform("inputResolution", glm::uvec2(previousFrame.mWidth, previousFrame.mHeight));
				populateShader.bindUniform("minLogLum", params.mMinLogLuminance);
				populateShader.bindUniform("inverseLogLumRange", 1.f / (params.mMaxLogLuminance - params.mMinLogLuminance + util::EP));
				auto imageBarrier1 = populateShader.bindImageTexture("previousHDRColor", previousFrame, types::shader::Access::Read);
				auto imageBarrier2 = populateShader.bindImageTexture("histogram", resourceManagers.mTextureManager.resolve(histogramHandle), types::shader::Access::ReadWrite);
				populateShader.dispatch({ std::ceil(previousFrame.mWidth / 16.f), std::ceil(previousFrame.mHeight / 16.f), 1 });
			}
		}, "Histogram populate");

		auto outputTexture = resourceManagers.mTextureManager.asyncLoad("Histogram Average", TextureBuilder{}
			.setDimension({ 1, 1, 0 })
			.setFormat(TextureFormat{
				types::texture::Target::Texture2D,
				types::texture::InternalFormats::R16_F,
				TextureFilter { types::texture::Filters::Nearest, types::texture::Filters::Nearest } 
			})
		);
		renderPasses.computePass([histogramHandle, outputTexture, params, previousFrameHDR](const ResourceManagers& resourceManagers, const ECS& ecs) {
			TRACY_GPUN("Histogram Average");
			if (!resourceManagers.mTextureManager.isValid(previousFrameHDR)) {
				return;
			}
			auto histogramAverageHandle = resourceManagers.mShaderManager.asyncLoad("HistogramAverage Shader", SourceShader::ConstructionArgs{
				{types::shader::Stage::Compute, "histogram_average.comp" }
				});
			if (resourceManagers.mTextureManager.isValid(outputTexture) && resourceManagers.mShaderManager.isValid(histogramAverageHandle)) {
				float dt = 1.f;
				if (auto frameStats = ecs.cGetComponent<FrameStatsComponent>()) {
					dt = std::clamp(dt * params.mTimeCoefficient, 0.f, 1.f);
				}

				auto& averageShader = resourceManagers.mShaderManager.resolveDefines(histogramAverageHandle, {});
				const auto& previousFrame = resourceManagers.mTextureManager.resolve(previousFrameHDR);
				averageShader.bindUniform("inputResolution", glm::uvec2(previousFrame.mWidth, previousFrame.mHeight));
				averageShader.bindUniform("minLogLum", params.mMinLogLuminance);
				averageShader.bindUniform("logLumRange", params.mMaxLogLuminance - params.mMinLogLuminance);
				averageShader.bindUniform("timeCoefficient", dt);
				auto imageBarrier1 = averageShader.bindImageTexture("histogram", resourceManagers.mTextureManager.resolve(histogramHandle), types::shader::Access::Read);
				auto imageBarrier2 = averageShader.bindImageTexture("dst", resourceManagers.mTextureManager.resolve(outputTexture), types::shader::Access::Write);
				averageShader.dispatch({ 1, 1, 1 });
			}
		}, "Histogram average");

		return outputTexture;
	}
}
