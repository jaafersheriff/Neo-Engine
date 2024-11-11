#pragma once

#include "ECS/ECS.hpp"
#include "Util/Profiler.hpp"

#include "DeferredPBRRenderComponent.hpp"

#include "ECS/Component/RenderingComponent/PhongRenderComponent.hpp"
#include "ECS/Component/RenderingComponent/OpaqueComponent.hpp"
#include "ECS/Component/RenderingComponent/AlphaTestComponent.hpp"

#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"
#include "ECS/Component/RenderingComponent/MaterialComponent.hpp"
#include "ECS/Component/CollisionComponent/CameraCulledComponent.hpp"

#include "ECS/Component/LightComponent/LightComponent.hpp"
#include "ECS/Component/LightComponent/DirectionalLightComponent.hpp"
#include "ECS/Component/LightComponent/PointLightComponent.hpp"

#include "Renderer/GLObjects/SourceShader.hpp"
#include "Renderer/GLObjects/ResolvedShaderInstance.hpp"

#include "ResourceManager/ResourceManagers.hpp"

#include "Renderer/FrameGraph/FrameGraph.hpp"

namespace DeferredPBR {

	inline FramebufferHandle createGbuffer(const ResourceManagers& resourceManagers, glm::uvec2 dimension) {
		return resourceManagers.mFramebufferManager.asyncLoad("Gbuffer",
			FramebufferBuilder{}
			.setSize(dimension)
			// AlbedoAO
			.attach(TextureFormat{ types::texture::Target::Texture2D, types::texture::InternalFormats::RGBA16_F })
			// NormalRoughness
			.attach(TextureFormat{ types::texture::Target::Texture2D, types::texture::InternalFormats::RGBA16_F })
			// Emissive Metalness
			.attach(TextureFormat{ types::texture::Target::Texture2D, types::texture::InternalFormats::RGBA16_F })
			// Depth
			.attach(TextureFormat{ types::texture::Target::Texture2D, types::texture::InternalFormats::D16 }),
			resourceManagers.mTextureManager
		);

	}

	template<typename... CompTs, typename... Deps>
	void drawGBuffer(
		FrameGraph& fg,
		const FramebufferHandle gbufferHandle,
		Viewport vp,
		const ResourceManagers& resourceManagers,
		const ECS::Entity cameraEntity,
		Deps... deps
	) {
		TRACY_ZONE();

		auto shaderHandle = resourceManagers.mShaderManager.asyncLoad("GBuffer Shader", SourceShader::ConstructionArgs{
			{ types::shader::Stage::Vertex, "model.vert"},
			{ types::shader::Stage::Fragment, "deferredpbr/gbuffer.frag" }
			});

		fg.pass(gbufferHandle, vp, vp, {}, shaderHandle, [cameraEntity](Pass& pass, const ResourceManagers& resourceManagers, const ECS& ecs) {
			MakeDefine(ALPHA_TEST);
			if constexpr ((std::is_same_v<AlphaTestComponent, CompTs> || ...)) {
				pass.setDefine(ALPHA_TEST);
			}
			const auto& view = ecs.getView<const DeferredPBRRenderComponent, const MeshComponent, const MaterialComponent, const SpatialComponent, const CompTs...>();
			for (auto entity : view) {
				// VFC
				if (auto* culled = ecs.cGetComponent<CameraCulledComponent>(entity)) {
					if (!culled->isInView(ecs, entity, cameraEntity)) {
						continue;
					}
				}

				ShaderDefinesFG drawDefines;
				UniformBuffer uniforms;

				uniforms.bindUniform("P", ecs.cGetComponent<CameraComponent>(cameraEntity)->getProj());
				uniforms.bindUniform("V", ecs.cGetComponent<SpatialComponent>(cameraEntity)->getView());

				const auto& material = view.get<const MaterialComponent>(entity);

				MakeDefine(ALBEDO_MAP);
				uniforms.bindUniform("albedo", material.mAlbedoColor);
				if (resourceManagers.mTextureManager.isValid(material.mAlbedoMap)) {
					drawDefines.set(ALBEDO_MAP);
					uniforms.bindTexture("albedoMap", material.mAlbedoMap);
				}
				MakeDefine(NORMAL_MAP);
				if (resourceManagers.mTextureManager.isValid(material.mNormalMap)) {
					drawDefines.set(NORMAL_MAP);
					uniforms.bindTexture("normalMap", material.mNormalMap);
				}
				uniforms.bindUniform("metalness", material.mMetallic);
				uniforms.bindUniform("roughness", material.mRoughness);
				MakeDefine(METAL_ROUGHNESS_MAP);
				if (resourceManagers.mTextureManager.isValid(material.mMetallicRoughnessMap)) {
					drawDefines.set(METAL_ROUGHNESS_MAP);
					uniforms.bindTexture("metalRoughnessMap", material.mMetallicRoughnessMap);
				}
				MakeDefine(OCCLUSION_MAP);
				if (resourceManagers.mTextureManager.isValid(material.mOcclusionMap)) {
					drawDefines.set(OCCLUSION_MAP);
					uniforms.bindTexture("occlusionMap", material.mOcclusionMap);
				}
				uniforms.bindUniform("emissiveFactor", material.mEmissiveFactor);
				MakeDefine(EMISSIVE);
				if (resourceManagers.mTextureManager.isValid(material.mEmissiveMap)) {
					drawDefines.set(EMISSIVE);
					uniforms.bindTexture("emissiveMap", material.mEmissiveMap);
				}

				const auto& mesh = resourceManagers.mMeshManager.resolve(view.get<const MeshComponent>(entity).mMeshHandle);
				MakeDefine(TANGENTS);
				if (mesh.hasVBO(types::mesh::VertexType::Tangent)) {
					drawDefines.set(TANGENTS);
				}

				const auto& drawSpatial = view.get<const SpatialComponent>(entity);
				uniforms.bindUniform("M", drawSpatial.getModelMatrix());
				uniforms.bindUniform("N", drawSpatial.getNormalMatrix());

				pass.drawCommand(view.get<const MeshComponent>(entity).mMeshHandle, uniforms, drawDefines);
			}
		}, deps...).mDebugName = "Gbuffer Pass";
	}


	struct GBufferDebugParameters {

		enum class DebugMode : uint8_t{
			Off,
			Normal,
			Albedo,
			AO,
			Roughness,
			Emissive,
			Metalness,
			Depth,
			COUNT
		};
		DebugMode mDebugMode = DebugMode::Off;

		bool imguiEditor() {
			static std::unordered_map<DebugMode, const char*> sDebugModeStrings = {
				{ DebugMode::Off, "Off"},
				{ DebugMode::Normal, "Normal"},
				{ DebugMode::Albedo, "Albedo"},
				{ DebugMode::AO, "AO"},
				{ DebugMode::Roughness, "Roughness"},
				{ DebugMode::Emissive, "Emissive"},
				{ DebugMode::Metalness, "Metalness"},
				{ DebugMode::Depth, "Depth"},
			};
			bool mod = false;
			if (ImGui::BeginCombo("Debug Mode", sDebugModeStrings[mDebugMode])) {
				for (int i = 0; i < static_cast<int>(DebugMode::COUNT); i++) {
					if (ImGui::Selectable(sDebugModeStrings[static_cast<DebugMode>(i)], mDebugMode == static_cast<DebugMode>(i))) {
						mDebugMode = static_cast<DebugMode>(i);
						mod = true;
					}
				}
				ImGui::EndCombo();
			}
			return mod;
		}
	};

	inline FramebufferHandle drawGBufferDebug(const ResourceManagers& resourceManagers, FramebufferHandle gbufferHandle, glm::uvec2 viewportSize, const GBufferDebugParameters& debugParameters) {
		TRACY_GPU();

		if (debugParameters.mDebugMode == GBufferDebugParameters::DebugMode::Off) {
			return NEO_INVALID_HANDLE;
		}

		auto outputHandle = resourceManagers.mFramebufferManager.asyncLoad("GBuffer Debug", FramebufferBuilder{}
			.setSize(viewportSize)
			.attach(TextureFormat{
				types::texture::Target::Texture2D,
				types::texture::InternalFormats::RGB8_UNORM
			}),
			resourceManagers.mTextureManager
		);
		auto shaderHandle = resourceManagers.mShaderManager.asyncLoad("GBufferDebug", SourceShader::ConstructionArgs{
			{ types::shader::Stage::Vertex, "quad.vert"},
			{ types::shader::Stage::Fragment, "deferredpbr/gbuffer_debug.frag" }
		});

		if (resourceManagers.mFramebufferManager.isValid(gbufferHandle) && resourceManagers.mFramebufferManager.isValid(outputHandle) && resourceManagers.mShaderManager.isValid(shaderHandle)) {
			auto output = resourceManagers.mFramebufferManager.resolve(outputHandle);
			output.bind();
			glViewport(0, 0, viewportSize.x, viewportSize.y);

			ShaderDefines defines;
			MakeDefine(NORMAL);
			MakeDefine(ALBEDO);
			MakeDefine(AO);
			MakeDefine(ROUGHNESS);
			MakeDefine(EMISSIVE);
			MakeDefine(METALNESS);
			MakeDefine(DEPTH);
			switch (debugParameters.mDebugMode) {
			case GBufferDebugParameters::DebugMode::Normal:
				defines.set(NORMAL);
				break;
			case GBufferDebugParameters::DebugMode::Albedo:
				defines.set(ALBEDO);
				break;
			case GBufferDebugParameters::DebugMode::AO:
				defines.set(AO);
				break;
			case GBufferDebugParameters::DebugMode::Roughness:
				defines.set(ROUGHNESS);
				break;
			case GBufferDebugParameters::DebugMode::Emissive:
				defines.set(EMISSIVE);
				break;
			case GBufferDebugParameters::DebugMode::Metalness:
				defines.set(METALNESS);
				break;
			case GBufferDebugParameters::DebugMode::Depth:
				defines.set(DEPTH);
				break;
			default:
				NEO_FAIL("Invalid debug mode");
				return NEO_INVALID_HANDLE;
			}

			auto& resolvedShader = resourceManagers.mShaderManager.resolveDefines(shaderHandle, defines);
			resolvedShader.bind();

			auto& gbuffer = resourceManagers.mFramebufferManager.resolve(gbufferHandle);
			resolvedShader.bindTexture("gAlbedoAO", resourceManagers.mTextureManager.resolve(gbuffer.mTextures[0]));
			resolvedShader.bindTexture("gNormalRoughness", resourceManagers.mTextureManager.resolve(gbuffer.mTextures[1]));
			resolvedShader.bindTexture("gEmissiveMetalness", resourceManagers.mTextureManager.resolve(gbuffer.mTextures[2]));
			resolvedShader.bindTexture("gDepth", resourceManagers.mTextureManager.resolve(gbuffer.mTextures[3]));

			resourceManagers.mMeshManager.resolve("quad").draw();
		}

		return outputHandle;
	}
}
