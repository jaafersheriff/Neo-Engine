#include "PBR/PBR.hpp"
#include "Engine/Engine.hpp"

#include "ECS/ECS.hpp"

#include "ECS/Component/CameraComponent/MainCameraComponent.hpp"
#include "ECS/Component/CameraComponent/FrustumComponent.hpp"
#include "ECS/Component/CameraComponent/FrustumFitReceiverComponent.hpp"
#include "ECS/Component/CameraComponent/FrustumFitSourceComponent.hpp"
#include "ECS/Component/EngineComponents/TagComponent.hpp"
#include "ECS/Component/HardwareComponent/ViewportDetailsComponent.hpp"
#include "ECS/Component/RenderingComponent/ShadowCasterShaderComponent.hpp"
#include "ECS/Component/RenderingComponent/SkyboxComponent.hpp"
#include "ECS/Component/SpatialComponent/RotationComponent.hpp"

#include "ECS/Systems/CameraSystems/CameraControllerSystem.hpp"
#include "ECS/Systems/CameraSystems/FrustaFittingSystem.hpp"
#include "ECS/Systems/TranslationSystems/RotationSystem.hpp"

#include "Renderer/RenderingSystems/PhongRenderer.hpp"
#include "Renderer/RenderingSystems/FXAARenderer.hpp"
#include "Renderer/RenderingSystems/ShadowMapRenderer.hpp"
#include "Renderer/RenderingSystems/SkyboxRenderer.hpp"
#include "Renderer/GLObjects/Framebuffer.hpp"
#include "Renderer/GLObjects/ResolvedShaderInstance.hpp"

#include "Loader/GLTFImporter.hpp"

#include "glm/gtc/matrix_transform.hpp"

using namespace neo;

namespace PBR {
	struct PBRLightComponent : public DirectionalLightComponent {
		PBRLightComponent(float s) :
			mStrength(s)
		{}

		virtual std::string getName() const override {
			return "PBRLightComponent";
		}

		virtual void imGuiEditor() override {
			ImGui::SliderFloat("Strength", &mStrength, 0.1f, 100.f);
		};

		float mStrength = 1.f;
	};

	template<typename... CompTs>
	void _drawPBR(const ECS& ecs, ECS::Entity cameraEntity, DebugMode debugMode, Texture* shadowMap) {
		TRACY_GPU();

		ShaderDefines passDefines({});

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

		const glm::mat4 P = ecs.cGetComponentAs<CameraComponent, PerspectiveCameraComponent>(cameraEntity)->getProj();
		const auto& cameraSpatial = ecs.cGetComponent<SpatialComponent>(cameraEntity);
		auto&& [lightEntity, _mainLight, pbrLight, light, lightSpatial] = *ecs.getSingleView<MainLightComponent, PBRLightComponent, LightComponent, SpatialComponent>();

		glm::mat4 L;
		const auto shadowCamera = ecs.getSingleView<ShadowCameraComponent, OrthoCameraComponent, SpatialComponent>();
		const bool shadowsEnabled = shadowMap && shadowCamera.has_value();
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

		MakeDefine(SKYBOX);
		auto skybox = ecs.cGetComponent<SkyboxComponent>();
		if (skybox) {
			passDefines.set(SKYBOX);
		}

		SourceShader* shader = Library::createSourceShader("PBR Shader", SourceShader::ConstructionArgs{
				{ ShaderStage::VERTEX, "model.vert"},
				{ ShaderStage::FRAGMENT, "pbr/pbr.frag" }
			});

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
			if (material.mAlbedoMap) {
				drawDefines.set(ALBEDO_MAP);
			}
			if (material.mNormalMap) {
				drawDefines.set(NORMAL_MAP);
			}
			if (material.mMetallicRoughnessMap) {
				drawDefines.set(METAL_ROUGHNESS_MAP);
			}
			if (material.mOcclusionMap) {
				drawDefines.set(OCCLUSION_MAP);
			}
			if (material.mEmissiveMap) {
				drawDefines.set(EMISSIVE);
			}

			const auto& mesh = view.get<const MeshComponent>(entity);
			MakeDefine(TANGENTS);
			if (mesh.mMesh->hasVBO(types::mesh::VertexType::Tangent)) {
				drawDefines.set(TANGENTS);
			}

			auto& resolvedShader = shader->getResolvedInstance(drawDefines);
			resolvedShader.bind();

			resolvedShader.bindUniform("albedo", material.mAlbedoColor);
			if (material.mAlbedoMap) {
				resolvedShader.bindTexture("albedoMap", *material.mAlbedoMap);
			}

			if (material.mNormalMap) {
				resolvedShader.bindTexture("normalMap", *material.mNormalMap);
			}

			resolvedShader.bindUniform("metalness", material.mMetallic);
			resolvedShader.bindUniform("roughness", material.mRoughness);
			if (material.mMetallicRoughnessMap) {
				resolvedShader.bindTexture("metalRoughnessMap", *material.mMetallicRoughnessMap);
			}

			if (material.mOcclusionMap) {
				resolvedShader.bindTexture("occlusionMap", *material.mOcclusionMap);
			}

			resolvedShader.bindUniform("emissiveFactor", material.mEmissiveFactor);
			if (material.mEmissiveMap) {
				resolvedShader.bindTexture("emissiveMap", *material.mEmissiveMap);
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
					resolvedShader.bindUniform("shadowMapResolution", glm::vec2(shadowMap->mWidth, shadowMap->mHeight));
					resolvedShader.bindTexture("shadowMap", *shadowMap);
				}
				if (skybox) {
					resolvedShader.bindTexture("skybox", *std::get<1>(*skybox).mSkybox);
				}
			}

			const auto& drawSpatial = view.get<const SpatialComponent>(entity);
			resolvedShader.bindUniform("M", drawSpatial.getModelMatrix());
			resolvedShader.bindUniform("N", drawSpatial.getNormalMatrix());

			view.get<const MeshComponent>(entity).mMesh->draw();
		}
	}

	IDemo::Config Demo::getConfig() const {
		IDemo::Config config;
		config.name = "PBR Demo";
		return config;
	}

	void Demo::init(ECS& ecs) {

		/* Camera */
		{
			auto entity = ecs.createEntity();
			ecs.addComponent<TagComponent>(entity, "Camera");
			ecs.addComponent<SpatialComponent>(entity, glm::vec3(0.05f, 0.03f, 0.0f), glm::vec3(1.f));
			ecs.addComponent<PerspectiveCameraComponent>(entity, 0.1f, 35.f, 45.f);
			ecs.addComponent<CameraControllerComponent>(entity, 0.4f, 15.f);
			ecs.addComponent<MainCameraComponent>(entity);
			ecs.addComponent<FrustumComponent>(entity);
			ecs.addComponent<FrustumFitSourceComponent>(entity);
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
		}
		{
			auto shadowCam = ecs.createEntity();
			ecs.addComponent<TagComponent>(shadowCam, "Shadow Camera");
			ecs.addComponent<OrthoCameraComponent>(shadowCam, -1.f, 1000.f, -100.f, 100.f, -100.f, 100.f);
			ecs.addComponent<ShadowCameraComponent>(shadowCam);
			ecs.addComponent<FrustumComponent>(shadowCam);
			ecs.addComponent<SpatialComponent>(shadowCam);
			ecs.addComponent<FrustumFitReceiverComponent>(shadowCam, 1.f);
		}

		// Dialectric spheres
		static float numSpheres = 8;
		for (int i = 0; i < numSpheres; i++) {
			auto entity = ecs.createEntity();
			ecs.addComponent<SpatialComponent>(entity, glm::vec3(-2.f + i, 1.f, 0.f), glm::vec3(0.3f));
			auto mesh = Library::getMesh("sphere");
			ecs.addComponent<MeshComponent>(entity, mesh);
			ecs.addComponent<BoundingBoxComponent>(entity, mesh->mMin, mesh->mMax);
			ecs.addComponent<OpaqueComponent>(entity);
			auto material = ecs.addComponent<MaterialComponent>(entity);
			material->mAlbedoColor = glm::vec4(1, 0, 0, 1);
			material->mMetallic = 0.f;
			material->mRoughness = 1.f - i / numSpheres;
			ecs.addComponent<ShadowCasterShaderComponent>(entity);
		}
		// Conductive spheres
		for (int i = 0; i < numSpheres; i++) {
			auto entity = ecs.createEntity();
			ecs.addComponent<SpatialComponent>(entity, glm::vec3(-2.f + i, 1.f, -1.5f), glm::vec3(0.3f));
			auto mesh = Library::getMesh("sphere");
			ecs.addComponent<MeshComponent>(entity, mesh);
			ecs.addComponent<BoundingBoxComponent>(entity, mesh->mMin, mesh->mMax);
			ecs.addComponent<OpaqueComponent>(entity);
			auto material = ecs.addComponent<MaterialComponent>(entity);
			material->mAlbedoColor = glm::vec4(0.944f, 0.776f, 0.373f, 1);
			material->mMetallic = 1.f;
			material->mRoughness = 1.f - i / numSpheres;
			ecs.addComponent<ShadowCasterShaderComponent>(entity);
		}
		// Emissive sphere
		{
			auto entity = ecs.createEntity();
			ecs.addComponent<SpatialComponent>(entity, glm::vec3(0.f, 1.f, -0.75f), glm::vec3(0.3f));
			auto mesh = Library::getMesh("sphere");
			ecs.addComponent<MeshComponent>(entity, mesh);
			ecs.addComponent<BoundingBoxComponent>(entity, mesh->mMin, mesh->mMax);
			ecs.addComponent<OpaqueComponent>(entity);
			auto material = ecs.addComponent<MaterialComponent>(entity);
			material->mAlbedoColor = glm::vec4(1.f);
			material->mMetallic = 0.f;
			material->mRoughness = 0.f;
			material->mEmissiveFactor = glm::vec3(100.f);
			ecs.addComponent<ShadowCasterShaderComponent>(entity);
		}

		{
			auto skybox = ecs.createEntity();
			ecs.addComponent<SkyboxComponent>(skybox, Library::loadCubemap("Skybox", {
				"envmap_miramar/miramar_ft.tga",
				"envmap_miramar/miramar_bk.tga",
				"envmap_miramar/miramar_up.tga",
				"envmap_miramar/miramar_dn.tga",
				"envmap_miramar/miramar_rt.tga",
				"envmap_miramar/miramar_lf.tga",
			}));
		}

		{
			GLTFImporter::Node helmet = Loader::loadGltfScene("DamagedHelmet/DamagedHelmet.gltf", glm::translate(glm::mat4(1.f), glm::vec3(0.f, 2.5f, -0.5f))).mMeshNodes[0];
			auto entity = ecs.createEntity();
			if (!helmet.mName.empty()) {
				ecs.addComponent<TagComponent>(entity, helmet.mName);
			}
			ecs.addComponent<SpatialComponent>(entity, helmet.mSpatial);
			ecs.addComponent<MeshComponent>(entity, helmet.mMesh);
			ecs.addComponent<BoundingBoxComponent>(entity, helmet.mMesh->mMin, helmet.mMesh->mMax);
			if (helmet.mAlphaMode == GLTFImporter::Node::AlphaMode::Opaque) {
				ecs.addComponent<OpaqueComponent>(entity);
			}
			else if (helmet.mAlphaMode == GLTFImporter::Node::AlphaMode::AlphaTest) {
				ecs.addComponent<AlphaTestComponent>(entity);
			}
			ecs.addComponent<MaterialComponent>(entity, helmet.mMaterial);
			ecs.addComponent<RotationComponent>(entity, glm::vec3(0.f, 0.5f, 0.f));
			ecs.addComponent<ShadowCasterShaderComponent>(entity);
		}

		{
			GLTFImporter::Node bust = Loader::loadGltfScene("fblock.gltf", glm::scale(glm::translate(glm::mat4(1.f), glm::vec3(-5.f, 2.5f, -0.5f)), glm::vec3(2.f))).mMeshNodes[0];
			auto entity = ecs.createEntity();
			ecs.addComponent<TagComponent>(entity, "Bust");
			auto spatial = ecs.addComponent<SpatialComponent>(entity, bust.mSpatial);
			spatial->setLookDir(glm::vec3(0.f, 0.4f, 0.1f));
			ecs.addComponent<MeshComponent>(entity, bust.mMesh);
			ecs.addComponent<BoundingBoxComponent>(entity, bust.mMesh->mMin, bust.mMesh->mMax);
			ecs.addComponent<OpaqueComponent>(entity);
			ecs.addComponent<MaterialComponent>(entity, bust.mMaterial);
			ecs.addComponent<RotationComponent>(entity, glm::vec3(0.f, 0.5f, 0.f));
			ecs.addComponent<ShadowCasterShaderComponent>(entity);
		}


		{
			//GLTFImporter::Scene _scene = Loader::loadGltfScene("NormalTangentTest/NormalTangentTest.gltf");
			GLTFImporter::Scene _scene = Loader::loadGltfScene("Sponza/Sponza.gltf", glm::scale(glm::mat4(1.f), glm::vec3(200.f)));
			for (auto& node : _scene.mMeshNodes) {
				auto entity = ecs.createEntity();
				if (!node.mName.empty()) {
					ecs.addComponent<TagComponent>(entity, node.mName);
				}
				ecs.addComponent<SpatialComponent>(entity, node.mSpatial);
				ecs.addComponent<MeshComponent>(entity, node.mMesh);
				ecs.addComponent<BoundingBoxComponent>(entity, node.mMesh->mMin, node.mMesh->mMax);
				if (node.mAlphaMode == GLTFImporter::Node::AlphaMode::Opaque) {
					ecs.addComponent<OpaqueComponent>(entity);
				}
				else if (node.mAlphaMode == GLTFImporter::Node::AlphaMode::AlphaTest) {
					ecs.addComponent<AlphaTestComponent>(entity);
				}
				ecs.addComponent<MaterialComponent>(entity, node.mMaterial);
				ecs.addComponent<ShadowCasterShaderComponent>(entity);
			}
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

	void Demo::update(ECS& ecs) {
		NEO_UNUSED(ecs);
	}

	void Demo::render(const ECS& ecs, Framebuffer& backbuffer) {
		const auto&& [cameraEntity, _, cameraSpatial] = *ecs.getSingleView<MainCameraComponent, SpatialComponent>();

		auto viewport = std::get<1>(*ecs.cGetComponent<ViewportDetailsComponent>());
		auto sceneTarget = Library::getPooledFramebuffer(PooledFramebufferDetails{ viewport.mSize, {
			TextureFormat {
				types::texture::Target::Texture2D,
				types::texture::InternalFormats::RGB16_F,
			},
			TextureFormat {
				types::texture::Target::Texture2D,
				types::texture::InternalFormats::D16,
			}
		} }, "Scene target");

		auto shadowMap = Library::getPooledFramebuffer({ glm::uvec2(4096, 4096), { 
			TextureFormat {
				types::texture::Target::Texture2D,
				types::texture::InternalFormats::D16,
			}
		} }, "Shadow map");
		if (mDrawShadows) {
			shadowMap->clear(glm::uvec4(0.f, 0.f, 0.f, 0.f), types::framebuffer::ClearFlagBits::Depth);
			drawShadows<OpaqueComponent>(*shadowMap, ecs);
			drawShadows<AlphaTestComponent>(*shadowMap, ecs);
		}


		glm::vec3 clearColor = getConfig().clearColor;

		sceneTarget->bind();
		sceneTarget->clear(glm::vec4(clearColor, 1.f), types::framebuffer::ClearFlagBits::Color | types::framebuffer::ClearFlagBits::Depth);
		glViewport(0, 0, viewport.mSize.x, viewport.mSize.y);

		drawSkybox(ecs, cameraEntity);

		_drawPBR<OpaqueComponent>(ecs, cameraEntity, mDebugMode, mDrawShadows ? shadowMap->mTextures[0] : nullptr);
		_drawPBR<AlphaTestComponent>(ecs, cameraEntity, mDebugMode, mDrawShadows ? shadowMap->mTextures[0] : nullptr);

		backbuffer.bind();
		backbuffer.clear(glm::vec4(clearColor, 1.f), types::framebuffer::ClearFlagBits::Color);
		drawFXAA(glm::uvec2(backbuffer.mTextures[0]->mWidth, backbuffer.mTextures[0]->mHeight), *sceneTarget->mTextures[0]);
		// Don't forget the depth. Because reasons.
		glBlitNamedFramebuffer(sceneTarget->mFBOID, backbuffer.mFBOID,
			0, 0, viewport.mSize.x, viewport.mSize.y,
			0, 0, viewport.mSize.x, viewport.mSize.y,
			GL_DEPTH_BUFFER_BIT,
			GL_NEAREST
		);

	}

	void Demo::destroy() {
	}
}
