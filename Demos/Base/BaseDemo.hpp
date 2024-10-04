#pragma once

#include "DemoInfra/IDemo.hpp"
#include "ECS/Component/RenderingComponent/PhongRenderComponent.hpp"

#include "ECS/Component/RenderingComponent/AlphaTestComponent.hpp"
#include "ECS/Component/RenderingComponent/OpaqueComponent.hpp"
#include "ECS/Component/RenderingComponent/PhongRenderComponent.hpp"
#include "ECS/Component/RenderingComponent/MeshComponent.hpp"
#include "ECS/Component/RenderingComponent/TransparentComponent.hpp"
#include "ECS/Component/RenderingComponent/MaterialComponent.hpp"

#include "ECS/Component/CameraComponent/ShadowCameraComponent.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"
#include "ECS/Component/CollisionComponent/CameraCulledComponent.hpp"

#include "ECS/Component/LightComponent/MainLightComponent.hpp"
#include "ECS/Component/LightComponent/LightComponent.hpp"
#include "ECS/Component/LightComponent/DirectionalLightComponent.hpp"
#include "ECS/Component/LightComponent/PointLightComponent.hpp"




#include "Renderer/Decl.hpp"

using namespace neo;

namespace Base {

	class Demo : public IDemo {
	public:
		virtual IDemo::Config getConfig() const override;
		virtual void init(ECS& ecs, ResourceManagers& resourceManagers) override;
		virtual void update(ECS& ecs, ResourceManagers& resourceManagers) override;
		virtual void render(const ResourceManagers& resourceManagers, const ECS& ecs, Framebuffer& backbuffer) override;
		virtual void destroy() override;
		virtual void imGuiEditor(ECS& ecs, ResourceManagers& resourceManagers) override;

	private:
		bool useDecl = true;

		template<typename... CompTs>
		void _drawPhong(Decl& decl, FramebufferHandle target, glm::uvec4 viewport, const ResourceManagers& resourceManagers, const ECS& ecs, const ECS::Entity cameraEntity, const ShaderDefines& inDefines = {}) {
			TRACY_GPU();

			RenderPass& renderPass = decl.declareRenderPass(target, viewport);

			renderPass.shaderHandle = resourceManagers.mShaderManager.asyncLoad("Phong Shader",
				SourceShader::ConstructionArgs{
					{ types::shader::Stage::Vertex, "model.vert"},
					{ types::shader::Stage::Fragment, "phong.frag" }
				}
			);

			{
				TRACY_ZONEN("Base");
				NEO_ASSERT(inDefines.mDefines.size() == 0, "TODO");
				MakeDefine(ALPHA_TEST);
				MakeDefine(TRANSPARENT);
				if constexpr ((std::is_same_v<AlphaTestComponent, CompTs> || ...)) {
					renderPass.passDefines.set(ALPHA_TEST);
				}
				if constexpr ((std::is_same_v<TransparentComponent, CompTs> || ...)) {
					renderPass.passDefines.set(TRANSPARENT);
					renderPass.state.blendState = BlendState::Enabled;
				}
			}

			{
				TRACY_ZONEN("Camera");
				const auto& cameraSpatial = ecs.cGetComponent<SpatialComponent>(cameraEntity);
				MakeUniform(P);
				MakeUniform(V);
				MakeUniform(camPos);
				renderPass.passUniforms.set(P, ecs.cGetComponent<CameraComponent>(cameraEntity)->getProj());
				renderPass.passUniforms.set(V, cameraSpatial->getView());
				renderPass.passUniforms.set(camPos, cameraSpatial->getPosition());
			}

			auto&& [lightEntity, _lightLight, light, lightSpatial] = *ecs.getSingleView<MainLightComponent, LightComponent, SpatialComponent>();
			const bool shadowsEnabled =
				ecs.has<DirectionalLightComponent>(lightEntity)
				&& ecs.has<CameraComponent>(lightEntity)
				&& ecs.has<ShadowCameraComponent>(lightEntity)
				&& resourceManagers.mTextureManager.isValid(ecs.cGetComponent<ShadowCameraComponent>(lightEntity)->mShadowMap);
			{
				TRACY_ZONEN("Light");

				MakeDefine(ENABLE_SHADOWS);
				MakeUniform(L);
				MakeUniform(shadowMap);
				if (shadowsEnabled) {
					renderPass.passDefines.set(ENABLE_SHADOWS);

					const auto& shadowCamera = *ecs.cGetComponent<CameraComponent>(lightEntity);
					static glm::mat4 biasMatrix(
						0.5f, 0.0f, 0.0f, 0.0f,
						0.0f, 0.5f, 0.0f, 0.0f,
						0.0f, 0.0f, 0.5f, 0.0f,
						0.5f, 0.5f, 0.5f, 1.0f);
					renderPass.passUniforms.set(L, biasMatrix * shadowCamera.getProj() * lightSpatial.getView());
					renderPass.passTextures.set(shadowMap, ecs.cGetComponent<ShadowCameraComponent>(lightEntity)->mShadowMap);
				}
				bool directionalLight = ecs.has<DirectionalLightComponent>(lightEntity);
				bool pointLight = ecs.has<PointLightComponent>(lightEntity);
				MakeDefine(DIRECTIONAL_LIGHT);
				MakeDefine(POINT_LIGHT);
				MakeUniform(lightDir);
				MakeUniform(lightPos);
				MakeUniform(lightRadiance);
				MakeUniform(lightCol);
				renderPass.passUniforms.set(lightCol, light.mColor);
				renderPass.passUniforms.set(lightRadiance, light.mIntensity);
				if (directionalLight || shadowsEnabled) {
					renderPass.passDefines.set(DIRECTIONAL_LIGHT);
					renderPass.passUniforms.set(lightDir, -lightSpatial.getLookDir());
				}
				else if (pointLight) {
					renderPass.passDefines.set(POINT_LIGHT);
					renderPass.passUniforms.set(lightPos, lightSpatial.getPosition());
				}
				else {
					NEO_FAIL("Phong light needs a directional or point light component");
				}
			}

			// No transparency sorting on the view, because I'm lazy, and this is stinky phong renderer
			{
				TRACY_ZONEN("View Iter");
				const auto& view = ecs.getView<const PhongRenderComponent, const MeshComponent, const MaterialComponent, const SpatialComponent, const CompTs...>();
				for (auto entity : view) {
					// VFC
					if (auto* culled = ecs.cGetComponent<CameraCulledComponent>(entity)) {
						if (!culled->isInView(ecs, entity, cameraEntity)) {
							continue;
						}
					}

					auto& draw = renderPass.declareDraw();

					const auto& material = view.get<const MaterialComponent>(entity);
					MakeDefine(ALBEDO_MAP);
					MakeUniform(albedo);
					MakeUniform(albedoMap);
					if (resourceManagers.mTextureManager.isValid(material.mAlbedoMap)) {
						draw.drawDefines.set(ALBEDO_MAP);
						draw.textures.set(albedoMap, material.mAlbedoMap);
					}
					draw.uniforms.set(albedo, material.mAlbedoColor);

					MakeDefine(NORMAL_MAP);
					MakeUniform(normalMap);
					if (resourceManagers.mTextureManager.isValid(material.mNormalMap)) {
						draw.drawDefines.set(NORMAL_MAP);
						draw.textures.set(normalMap, material.mNormalMap);
					}

					const auto& drawSpatial = view.get<const SpatialComponent>(entity);
					MakeUniform(M);
					MakeUniform(N);
					draw.uniforms.set(M, drawSpatial.getModelMatrix());
					draw.uniforms.set(N, drawSpatial.getNormalMatrix());

					draw.mesh = view.get<const MeshComponent>(entity).mMeshHandle;
				}
			}
		}

	};
}
