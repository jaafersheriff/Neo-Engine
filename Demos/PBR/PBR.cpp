#include "PBR/PBR.hpp"
#include "Engine/Engine.hpp"

#include "ECS/ECS.hpp"

#include "ECS/Component/CameraComponent/MainCameraComponent.hpp"
#include "ECS/Component/CameraComponent/FrustumComponent.hpp"
#include "ECS/Component/CameraComponent/FrustumFitReceiverComponent.hpp"
#include "ECS/Component/CameraComponent/FrustumFitSourceComponent.hpp"
#include "ECS/Component/EngineComponents/PinnedComponent.hpp"
#include "ECS/Component/EngineComponents/TagComponent.hpp"
#include "ECS/Component/HardwareComponent/ViewportDetailsComponent.hpp"
#include "ECS/Component/RenderingComponent/ShadowCasterRenderComponent.hpp"
#include "ECS/Component/RenderingComponent/SkyboxComponent.hpp"
#include "ECS/Component/SpatialComponent/RotationComponent.hpp"

#include "ECS/Systems/CameraSystems/CameraControllerSystem.hpp"
#include "ECS/Systems/CameraSystems/FrustaFittingSystem.hpp"
#include "ECS/Systems/TranslationSystems/RotationSystem.hpp"

#include "PBR/IBLComponent.hpp"
#include "PBR/ConvolveRenderer.hpp"

#include "Renderer/RenderingSystems/PhongRenderer.hpp"
#include "Renderer/RenderingSystems/ShadowMapRenderer.hpp"
#include "Renderer/RenderingSystems/SkyboxRenderer.hpp"
#include "Renderer/GLObjects/Framebuffer.hpp"
#include "Renderer/GLObjects/ResolvedShaderInstance.hpp"

#include "Loader/GLTFImporter.hpp"

#include "glm/gtc/matrix_transform.hpp"

using namespace neo;

namespace PBR {
	START_COMPONENT(PBRLightComponent);
		PBRLightComponent(float s) :
			mStrength(s)
		{}

		virtual void imGuiEditor() override {
			ImGui::SliderFloat("Strength", &mStrength, 0.1f, 100.f);
		};

		float mStrength = 1.f;
	END_COMPONENT();

	template<typename... CompTs>
	void _drawPBR(const ResourceManagers& resourceManagers, const ECS& ecs, ECS::Entity cameraEntity, DebugMode debugMode, TextureHandle shadowMapHandle, std::optional<const IBLComponent> ibl) {
		TRACY_GPU();

		ShaderDefines passDefines({});

		auto pbrShaderHandle = resourceManagers.mShaderManager.asyncLoad("PBR Shader", SourceShader::ConstructionArgs{
			{ types::shader::Stage::Vertex, "model.vert"},
			{ types::shader::Stage::Fragment, "pbr/pbr.frag" }
		});
		if (!resourceManagers.mShaderManager.isValid(pbrShaderHandle)) {
			return;
		}

		MakeDefine(DEBUG_ALBEDO);
		MakeDefine(DEBUG_METAL_ROUGHNESS);
		MakeDefine(DEBUG_EMISSIVE);
		MakeDefine(DEBUG_NORMALS);
		MakeDefine(DEBUG_DIFFUSE);
		MakeDefine(DEBUG_SPECULAR);
		switch (debugMode) {
		case DebugMode::Albedo:
			passDefines.set(DEBUG_ALBEDO);
			break;
		case DebugMode::MetalRoughness:
			passDefines.set(DEBUG_METAL_ROUGHNESS);
			break;
		case DebugMode::Emissives:
			passDefines.set(DEBUG_EMISSIVE);
			break;
		case DebugMode::Normals:
			passDefines.set(DEBUG_NORMALS);
			break;
		case DebugMode::Diffuse:
			passDefines.set(DEBUG_DIFFUSE);
			break;
		case DebugMode::Specular:
			passDefines.set(DEBUG_SPECULAR);
			break;
		case DebugMode::Off:
		default:
			break;
		}

		bool containsAlphaTest = false;
		MakeDefine(ALPHA_TEST);
		if constexpr ((std::is_same_v<AlphaTestComponent, CompTs> || ...)) {
			containsAlphaTest = true;
			passDefines.set(ALPHA_TEST);
		}

		const glm::mat4 P = ecs.cGetComponent<CameraComponent>(cameraEntity)->getProj();
		const auto& cameraSpatial = ecs.cGetComponent<SpatialComponent>(cameraEntity);
		auto&& [lightEntity, _mainLight, pbrLight, light, lightSpatial] = *ecs.getSingleView<MainLightComponent, PBRLightComponent, LightComponent, SpatialComponent>();

		glm::mat4 L;
		const auto shadowCamera = ecs.getSingleView<ShadowCameraComponent, CameraComponent, SpatialComponent>();
		const bool shadowsEnabled = resourceManagers.mTextureManager.isValid(shadowMapHandle) && shadowCamera.has_value();
		MakeDefine(ENABLE_SHADOWS);
		if (shadowsEnabled) {
			passDefines.set(ENABLE_SHADOWS);
			const auto& [_, __, shadowOrtho, shadowCameraSpatial] = *shadowCamera;
			static glm::mat4 biasMatrix(
				0.5f, 0.0f, 0.0f, 0.0f,
				0.0f, 0.5f, 0.0f, 0.0f,
				0.0f, 0.0f, 0.5f, 0.0f,
				0.5f, 0.5f, 0.5f, 1.0f);
			L = biasMatrix * shadowOrtho.getProj() * shadowCameraSpatial.getView();
		}

		bool directionalLight = ecs.has<DirectionalLightComponent>(lightEntity);
		bool pointLight = ecs.has<PointLightComponent>(lightEntity);
		glm::vec3 attenuation(0.f);
		MakeDefine(DIRECTIONAL_LIGHT);
		MakeDefine(POINT_LIGHT);
		if (directionalLight) {
			passDefines.set(DIRECTIONAL_LIGHT);
		}
		else if (pointLight) {
			attenuation = ecs.cGetComponent<PointLightComponent>(lightEntity)->mAttenuation;
			passDefines.set(POINT_LIGHT);
		}
		else {
			NEO_FAIL("Phong light needs a directional or point light component");
		}

		MakeDefine(IBL);
		if (ibl && resourceManagers.mTextureManager.isValid(ibl->mConvolvedSkybox) && resourceManagers.mTextureManager.isValid(ibl->mDFGLut)) {
			passDefines.set(IBL);
		}

		ShaderDefines drawDefines(passDefines);
		const auto& view = ecs.getView<const MeshComponent, const MaterialComponent, const SpatialComponent, const CompTs...>();
		for (auto entity : view) {
			// VFC
			if (auto* culled = ecs.cGetComponent<CameraCulledComponent>(entity)) {
				if (!culled->isInView(ecs, entity, cameraEntity)) {
					continue;
				}
			}

			if (containsAlphaTest) {
				NEO_ASSERT(!ecs.has<OpaqueComponent>(entity), "Entity has opaque and alpha test component?");
			}

			drawDefines.reset();
			const auto& material = view.get<const MaterialComponent>(entity);
			MakeDefine(ALBEDO_MAP);
			MakeDefine(NORMAL_MAP);
			MakeDefine(METAL_ROUGHNESS_MAP);
			MakeDefine(OCCLUSION_MAP);
			MakeDefine(EMISSIVE);
			if (resourceManagers.mTextureManager.isValid(material.mAlbedoMap)) {
				drawDefines.set(ALBEDO_MAP);
			}
			if (resourceManagers.mTextureManager.isValid(material.mNormalMap)) {
				drawDefines.set(NORMAL_MAP);
			}
			if (resourceManagers.mTextureManager.isValid(material.mMetallicRoughnessMap)) {
				drawDefines.set(METAL_ROUGHNESS_MAP);
			}
			if (resourceManagers.mTextureManager.isValid(material.mOcclusionMap)) {
				drawDefines.set(OCCLUSION_MAP);
			}
			if (resourceManagers.mTextureManager.isValid(material.mEmissiveMap)) {
				drawDefines.set(EMISSIVE);
			}

			const auto& mesh = resourceManagers.mMeshManager.resolve(view.get<const MeshComponent>(entity).mMeshHandle);
			MakeDefine(TANGENTS);
			if (mesh.hasVBO(types::mesh::VertexType::Tangent)) {
				drawDefines.set(TANGENTS);
			}

			auto& resolvedShader = resourceManagers.mShaderManager.resolveDefines(pbrShaderHandle, drawDefines);
			resolvedShader.bind();

			resolvedShader.bindUniform("albedo", material.mAlbedoColor);
			if (resourceManagers.mTextureManager.isValid(material.mAlbedoMap)) {
				resolvedShader.bindTexture("albedoMap", resourceManagers.mTextureManager.resolve(material.mAlbedoMap));
			}

			if (resourceManagers.mTextureManager.isValid(material.mNormalMap)) {
				resolvedShader.bindTexture("normalMap", resourceManagers.mTextureManager.resolve(material.mNormalMap));
			}

			resolvedShader.bindUniform("metalness", material.mMetallic);
			resolvedShader.bindUniform("roughness", material.mRoughness);
			if (resourceManagers.mTextureManager.isValid(material.mMetallicRoughnessMap)) {
				resolvedShader.bindTexture("metalRoughnessMap", resourceManagers.mTextureManager.resolve(material.mMetallicRoughnessMap));
			}

			if (resourceManagers.mTextureManager.isValid(material.mOcclusionMap)) {
				resolvedShader.bindTexture("occlusionMap", resourceManagers.mTextureManager.resolve(material.mOcclusionMap));
			}

			resolvedShader.bindUniform("emissiveFactor", material.mEmissiveFactor);
			if (resourceManagers.mTextureManager.isValid(material.mEmissiveMap)) {
				resolvedShader.bindTexture("emissiveMap", resourceManagers.mTextureManager.resolve(material.mEmissiveMap));
			}

			// UBO candidates
			{
				resolvedShader.bindUniform("P", P);
				resolvedShader.bindUniform("V", cameraSpatial->getView());
				resolvedShader.bindUniform("camPos", cameraSpatial->getPosition());
				resolvedShader.bindUniform("camDir", cameraSpatial->getLookDir());
				resolvedShader.bindUniform("lightRadiance", glm::vec4(light.mColor, pbrLight.mStrength));
				if (directionalLight || shadowsEnabled) {
					resolvedShader.bindUniform("lightDir", -lightSpatial.getLookDir());
				}
				if (pointLight) {
					resolvedShader.bindUniform("lightPos", lightSpatial.getPosition());
					resolvedShader.bindUniform("lightAtt", attenuation);
				}
				if (shadowsEnabled) {
					resolvedShader.bindUniform("L", L);
					auto& shadowMap = resourceManagers.mTextureManager.resolve(shadowMapHandle);
					resolvedShader.bindUniform("shadowMapResolution", glm::vec2(shadowMap.mWidth, shadowMap.mHeight));
					resolvedShader.bindTexture("shadowMap", shadowMap);
				}
				if (ibl) {
					const auto& iblTexture = resourceManagers.mTextureManager.resolve(ibl->mConvolvedSkybox);
					resolvedShader.bindTexture("dfgLUT", resourceManagers.mTextureManager.resolve(ibl->mDFGLut));
					resolvedShader.bindTexture("ibl", iblTexture);
					resolvedShader.bindUniform("iblMips", iblTexture.mFormat.mMipCount);
				}
			}

			const auto& drawSpatial = view.get<const SpatialComponent>(entity);
			resolvedShader.bindUniform("M", drawSpatial.getModelMatrix());
			resolvedShader.bindUniform("N", drawSpatial.getNormalMatrix());

			// Yikes
			if (material.mDoubleSided) {
				glDisable(GL_CULL_FACE);
			}
			else {
				glEnable(GL_CULL_FACE);
			}
			mesh.draw();
		}
	}

	IDemo::Config Demo::getConfig() const {
		IDemo::Config config;
		config.name = "PBR";
		return config;
	}

	void Demo::init(ECS& ecs, ResourceManagers& resourceManagers) {

		{
			auto entity = ecs.createEntity();
			ecs.addComponent<CameraControllerComponent>(entity, 0.4f, 15.f);
			ecs.addComponent<MainCameraComponent>(entity);
			ecs.addComponent<FrustumComponent>(entity);
			ecs.addComponent<FrustumFitSourceComponent>(entity);
			ecs.addComponent<PinnedComponent>(entity);
			ecs.addComponent<TagComponent>(entity, "Camera");
			ecs.addComponent<SpatialComponent>(entity, glm::vec3(0.05f, 0.03f, 0.0f), glm::vec3(1.f));
			ecs.addComponent<CameraComponent>(entity, 0.1f, 35.f, CameraComponent::Perspective{ 45.f, 1.f });
		}
		{
			auto lightEntity = ecs.createEntity();
			ecs.addComponent<TagComponent>(lightEntity, "Light");
			auto spat = ecs.addComponent<SpatialComponent>(lightEntity, glm::vec3(75.f, 200.f, 20.f));
			spat->setLookDir(glm::normalize(glm::vec3(-0.28f, -0.96f, -0.06f)));
			ecs.addComponent<LightComponent>(lightEntity, glm::vec3(1.f));
			ecs.addComponent<MainLightComponent>(lightEntity);
			ecs.addComponent<DirectionalLightComponent>(lightEntity);
			ecs.addComponent<PBRLightComponent>(lightEntity, 2.f);
			ecs.addComponent<PinnedComponent>(lightEntity);
		}
		{
			auto shadowCam = ecs.createEntity();
			ecs.addComponent<TagComponent>(shadowCam, "Shadow Camera");
			ecs.addComponent<CameraComponent>(shadowCam, -1.f, 1000.f, CameraComponent::Orthographic{ glm::vec2(-100.f, 100.f), glm::vec2(-100.f, 100.f) });
			ecs.addComponent<ShadowCameraComponent>(shadowCam);
			ecs.addComponent<FrustumComponent>(shadowCam);
			ecs.addComponent<SpatialComponent>(shadowCam);
			ecs.addComponent<FrustumFitReceiverComponent>(shadowCam, 1.f);
		}

		// Dialectric spheres
		static float numSpheres = 8;
		for (int i = 0; i < numSpheres; i++) {
			auto entity = ecs.createEntity();
			ecs.addComponent<SpatialComponent>(entity, glm::vec3(-2.f + i, 1.f, 0.f), glm::vec3(0.6f));
			ecs.addComponent<MeshComponent>(entity, HashedString("sphere"));
			ecs.addComponent<BoundingBoxComponent>(entity, glm::vec3(-0.5f), glm::vec3(0.5f));
			ecs.addComponent<OpaqueComponent>(entity);
			auto material = ecs.addComponent<MaterialComponent>(entity);
			material->mAlbedoColor = glm::vec4(1, 0, 0, 1);
			material->mMetallic = 0.f;
			material->mRoughness = 1.f - i / numSpheres;
			ecs.addComponent<ShadowCasterRenderComponent>(entity);
		}
		// Conductive spheres
		for (int i = 0; i < numSpheres; i++) {
			auto entity = ecs.createEntity();
			ecs.addComponent<SpatialComponent>(entity, glm::vec3(-2.f + i, 1.f, -1.5f), glm::vec3(0.6f));
			ecs.addComponent<MeshComponent>(entity, HashedString("sphere"));
			ecs.addComponent<BoundingBoxComponent>(entity, glm::vec3(-0.5f), glm::vec3(0.5f));
			ecs.addComponent<OpaqueComponent>(entity);
			auto material = ecs.addComponent<MaterialComponent>(entity);
			material->mAlbedoColor = glm::vec4(0.944f, 0.776f, 0.373f, 1);
			material->mMetallic = 1.f;
			material->mRoughness = 1.f - i / numSpheres;
			ecs.addComponent<ShadowCasterRenderComponent>(entity);
		}
		{
			auto icosahedron = ecs.createEntity();
			ecs.addComponent<TagComponent>(icosahedron, "Icosahedron");
			ecs.addComponent<SpatialComponent>(icosahedron, glm::vec3(-3.f, 4.0f, -0.5f), glm::vec3(1.f));
			ecs.addComponent<RotationComponent>(icosahedron, glm::vec3(0.f, 0.0f, 1.f));
			ecs.addComponent<MeshComponent>(icosahedron, HashedString("icosahedron"));
			ecs.addComponent<BoundingBoxComponent>(icosahedron, glm::vec3(-0.5f), glm::vec3(0.5f));
			ecs.addComponent<OpaqueComponent>(icosahedron);
			auto material = ecs.addComponent<MaterialComponent>(icosahedron);
			material->mAlbedoColor = glm::vec4(0.25f, 0.f, 1.f, 1);
			material->mMetallic = 1.f;
			material->mRoughness = 0.6f;
			ecs.addComponent<ShadowCasterRenderComponent>(icosahedron);
			ecs.addComponent<PinnedComponent>(icosahedron);
		}


		// Emissive sphere
		{
			auto entity = ecs.createEntity();
			ecs.addComponent<SpatialComponent>(entity, glm::vec3(0.f, 1.f, -0.75f), glm::vec3(0.6f));
			ecs.addComponent<MeshComponent>(entity, HashedString("sphere"));
			ecs.addComponent<BoundingBoxComponent>(entity, glm::vec3(-0.5f), glm::vec3(0.5f));
			ecs.addComponent<OpaqueComponent>(entity);
			auto material = ecs.addComponent<MaterialComponent>(entity);
			material->mAlbedoColor = glm::vec4(1.f);
			material->mMetallic = 0.f;
			material->mRoughness = 0.f;
			material->mEmissiveFactor = glm::vec3(100.f);
			ecs.addComponent<ShadowCasterRenderComponent>(entity);
		}

		{
			auto skybox = ecs.createEntity();
			ecs.addComponent<TagComponent>(skybox, "Skybox");
			ecs.addComponent<SkyboxComponent>(skybox, resourceManagers.mTextureManager.asyncLoad("Skybox", TextureFiles{ 
				{
					"envmap_miramar/miramar_ft.tga",
					"envmap_miramar/miramar_bk.tga",
					"envmap_miramar/miramar_up.tga",
					"envmap_miramar/miramar_dn.tga",
					"envmap_miramar/miramar_rt.tga",
					"envmap_miramar/miramar_lf.tga",
				}, 
				TextureFormat {
					types::texture::Target::TextureCube,
					types::texture::InternalFormats::RGBA8_UNORM, // This should change for .hdr?
					TextureFilter {
						types::texture::Filters::LinearMipmapLinear,
						types::texture::Filters::Linear
					},
					TextureWrap {
						types::texture::Wraps::Repeat,
						types::texture::Wraps::Repeat,
						types::texture::Wraps::Repeat
					},
					types::ByteFormats::UnsignedByte,
					6
				}
			}));
			ecs.addComponent<IBLComponent>(skybox);
			ecs.addComponent<PinnedComponent>(skybox);
		}

		{
			GLTFImporter::MeshNode helmet = Loader::loadGltfScene(resourceManagers, "DamagedHelmet/DamagedHelmet.gltf", glm::translate(glm::mat4(1.f), glm::vec3(0.f, 2.5f, -0.5f))).mMeshNodes[0];
			auto entity = ecs.createEntity();
			if (!helmet.mName.empty()) {
				ecs.addComponent<TagComponent>(entity, helmet.mName);
			}
			ecs.addComponent<SpatialComponent>(entity, helmet.mSpatial);
			ecs.addComponent<MeshComponent>(entity, helmet.mMeshHandle);
			ecs.addComponent<BoundingBoxComponent>(entity, helmet.mMin, helmet.mMax);
			if (helmet.mAlphaMode == GLTFImporter::MeshNode::AlphaMode::Opaque) {
				ecs.addComponent<OpaqueComponent>(entity);
			}
			else if (helmet.mAlphaMode == GLTFImporter::MeshNode::AlphaMode::AlphaTest) {
				ecs.addComponent<AlphaTestComponent>(entity);
			}
			ecs.addComponent<MaterialComponent>(entity, helmet.mMaterial);
			ecs.addComponent<RotationComponent>(entity, glm::vec3(0.f, 0.5f, 0.f));
			ecs.addComponent<ShadowCasterRenderComponent>(entity);
			ecs.addComponent<PinnedComponent>(entity);
		}

		{
			GLTFImporter::MeshNode bust = Loader::loadGltfScene(resourceManagers, "fblock.gltf", glm::scale(glm::translate(glm::mat4(1.f), glm::vec3(-5.f, 2.5f, -0.5f)), glm::vec3(2.f))).mMeshNodes[0];
			auto entity = ecs.createEntity();
			ecs.addComponent<TagComponent>(entity, "Bust");
			auto spatial = ecs.addComponent<SpatialComponent>(entity, bust.mSpatial);
			spatial->setLookDir(glm::vec3(0.f, 0.4f, 0.1f));
			ecs.addComponent<MeshComponent>(entity, bust.mMeshHandle);
			ecs.addComponent<BoundingBoxComponent>(entity, bust.mMin, bust.mMax);
			ecs.addComponent<OpaqueComponent>(entity);
			ecs.addComponent<MaterialComponent>(entity, bust.mMaterial);
			ecs.addComponent<RotationComponent>(entity, glm::vec3(0.f, 0.5f, 0.f));
			ecs.addComponent<ShadowCasterRenderComponent>(entity);
		}

		/* Systems - order matters! */
		ecs.addSystem<CameraControllerSystem>();
		ecs.addSystem<RotationSystem>();
		ecs.addSystem<FrustumSystem>();
		ecs.addSystem<FrustaFittingSystem>();
		ecs.addSystem<FrustumCullingSystem>();
	}

	void Demo::imGuiEditor(ECS& ecs) {
		NEO_UNUSED(ecs);
		ImGui::Checkbox("Shadows", &mDrawShadows);
		ImGui::Checkbox("IBL", &mDrawIBL);

		static std::unordered_map<DebugMode, const char*> sDebugModeStrings = {
			{DebugMode::Off, "Off"},
			{DebugMode::Albedo, "Albedo"},
			{DebugMode::MetalRoughness, "MetalRoughness"},
			{DebugMode::Emissives, "Emissive"},
			{DebugMode::Normals, "Normals"},
			{DebugMode::Diffuse, "Diffuse"},
			{DebugMode::Specular, "Specular"},
		};
		if (ImGui::BeginCombo("Debug Mode", sDebugModeStrings[mDebugMode])) {
			for (int i = 0; i < static_cast<int>(DebugMode::COUNT); i++) {
				if (ImGui::Selectable(sDebugModeStrings[static_cast<DebugMode>(i)], mDebugMode == static_cast<DebugMode>(i))) {
					mDebugMode = static_cast<DebugMode>(i);
				}
			}
			ImGui::EndCombo();
		}
	}

	void Demo::render(const ResourceManagers& resourceManagers, const ECS& ecs, Framebuffer& backbuffer) {
		convolveCubemap(resourceManagers, ecs);

		const auto& cameraTuple = ecs.getSingleView<MainCameraComponent, CameraComponent, SpatialComponent>();
		if (!cameraTuple) {
			return;
		}
		const auto& [cameraEntity, _, camera, cameraSpatial] = *cameraTuple;

		auto viewport = std::get<1>(*ecs.cGetComponent<ViewportDetailsComponent>());

		TextureHandle shadowTexture = NEO_INVALID_HANDLE;
		if (mDrawShadows) {
			shadowTexture = resourceManagers.mTextureManager.asyncLoad("Shadow map",
				TextureBuilder{}
					.setDimension(glm::u16vec3(2048, 2048, 0))
					.setFormat(TextureFormat{types::texture::Target::Texture2D, types::texture::InternalFormats::D16})
			);
			FramebufferHandle shadowTarget = resourceManagers.mFramebufferManager.asyncLoad(
				"Shadow map",
				FramebufferExternal{ { shadowTexture } },
				resourceManagers.mTextureManager
			);

			if (resourceManagers.mFramebufferManager.isValid(shadowTarget)) {
				auto& shadowMap = resourceManagers.mFramebufferManager.resolve(shadowTarget);
				shadowMap.clear(glm::uvec4(0.f, 0.f, 0.f, 0.f), types::framebuffer::AttachmentBit::Depth);
				drawShadows<OpaqueComponent>(resourceManagers, shadowMap, ecs);
				drawShadows<AlphaTestComponent>(resourceManagers, shadowMap, ecs);
			}
		}

		glm::vec3 clearColor = getConfig().clearColor;

		backbuffer.bind();
		backbuffer.clear(glm::vec4(clearColor, 1.f), types::framebuffer::AttachmentBit::Color | types::framebuffer::AttachmentBit::Depth);
		glViewport(0, 0, viewport.mSize.x, viewport.mSize.y);

		// Extract IBL
		std::optional<IBLComponent> ibl;
		const auto iblTuple = ecs.getSingleView<SkyboxComponent, IBLComponent>();
		if (iblTuple) {
			const auto& _ibl = std::get<2>(*iblTuple);
			if (_ibl.mConvolved && _ibl.mDFGGenerated) {
				ibl = _ibl;
			}
		}

		// Draw skybox
		if (ibl && ibl->mDebugIBL) {
			// Copy pasta of drawSkybox hehe
			auto iblDebugShaderHandle = resourceManagers.mShaderManager.asyncLoad("IBLDebug", SourceShader::ConstructionArgs{
				{ types::shader::Stage::Vertex, "skybox.vert"},
				{ types::shader::Stage::Fragment, "pbr/ibldebug.frag" }
				});
			if (resourceManagers.mShaderManager.isValid(iblDebugShaderHandle)) {
				glDisable(GL_CULL_FACE);
				glDisable(GL_DEPTH_TEST);
				glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
				auto& resolvedShader = resourceManagers.mShaderManager.resolveDefines(iblDebugShaderHandle, {});
				resolvedShader.bind();
				resolvedShader.bindUniform("P", camera.getProj());
				resolvedShader.bindUniform("V", cameraSpatial.getView());
				resolvedShader.bindTexture("cubeMap", resourceManagers.mTextureManager.resolve(ibl->mConvolvedSkybox));
				resolvedShader.bindUniform("mip", ibl->mDebugIBLMip);
				resourceManagers.mMeshManager.resolve(HashedString("cube")).draw();
				glEnable(GL_CULL_FACE);
				glEnable(GL_DEPTH_TEST);
			}
		}
		else {
			drawSkybox(resourceManagers, ecs, cameraEntity);
		}

		_drawPBR<OpaqueComponent>(resourceManagers, ecs, cameraEntity, mDebugMode, shadowTexture, mDrawIBL ? ibl : std::nullopt);
		_drawPBR<AlphaTestComponent>(resourceManagers, ecs, cameraEntity, mDebugMode, shadowTexture, mDrawIBL ? ibl : std::nullopt);
	}

	void Demo::destroy() {
	}
}
