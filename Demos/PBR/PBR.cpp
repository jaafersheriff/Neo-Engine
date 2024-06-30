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
#include "ECS/Component/RenderingComponent/IBLComponent.hpp"
#include "ECS/Component/RenderingComponent/SkyboxComponent.hpp"
#include "ECS/Component/SpatialComponent/RotationComponent.hpp"

#include "ECS/Systems/CameraSystems/CameraControllerSystem.hpp"
#include "ECS/Systems/CameraSystems/FrustaFittingSystem.hpp"
#include "ECS/Systems/TranslationSystems/RotationSystem.hpp"

#include "GBufferRenderer.hpp"
#include "DeferredPBRRenderer.hpp"

#include "Renderer/RenderingSystems/BloomRenderer.hpp"
#include "Renderer/RenderingSystems/ConvolveRenderer.hpp"
#include "Renderer/RenderingSystems/FXAARenderer.hpp"
#include "Renderer/RenderingSystems/ShadowMapRenderer.hpp"
#include "Renderer/RenderingSystems/SkyboxRenderer.hpp"
#include "Renderer/RenderingSystems/TonemapRenderer.hpp"
#include "Renderer/GLObjects/Framebuffer.hpp"
#include "Renderer/GLObjects/ResolvedShaderInstance.hpp"

#include "Loader/GLTFImporter.hpp"

#include "glm/gtc/matrix_transform.hpp"

using namespace neo;

namespace PBR {
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
			ecs.addComponent<LightComponent>(lightEntity, glm::vec3(0.978f, 0.903f, 0.714f), 15.f);
			ecs.addComponent<MainLightComponent>(lightEntity);
			ecs.addComponent<DirectionalLightComponent>(lightEntity);
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
			material->mRoughness = 1.f - i / (numSpheres-1);
			ecs.addComponent<ShadowCasterRenderComponent>(entity);
			ecs.addComponent<PBRDeferredComponent>(entity);
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
			material->mRoughness = 1.f - i / (numSpheres-1);
			ecs.addComponent<ShadowCasterRenderComponent>(entity);
			ecs.addComponent<PBRDeferredComponent>(entity);
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
			material->mMetallic = 0.f;
			material->mRoughness = 0.15f;
			ecs.addComponent<ShadowCasterRenderComponent>(icosahedron);
			ecs.addComponent<PinnedComponent>(icosahedron);
			ecs.addComponent<PBRDeferredComponent>(icosahedron);
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
			ecs.addComponent<PBRDeferredComponent>(entity);
		}

		{
			auto skybox = ecs.createEntity();
			ecs.addComponent<TagComponent>(skybox, "Skybox");
			ecs.addComponent<SkyboxComponent>(skybox, resourceManagers.mTextureManager.asyncLoad("hdr skybox", TextureFiles{
				{"metro_noord_2k.hdr" } ,
				TextureFormat{
					types::texture::Target::Texture2D,
					types::texture::InternalFormats::RGBA16_F,
					TextureFilter {
						types::texture::Filters::LinearMipmapLinear,
						types::texture::Filters::Linear
					},
					TextureWrap {
						types::texture::Wraps::Repeat,
						types::texture::Wraps::Repeat,
						types::texture::Wraps::Repeat
					},
					types::ByteFormats::Float,
					7
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
			ecs.addComponent<PBRDeferredComponent>(entity);
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
			ecs.addComponent<PBRDeferredComponent>(entity);
		}
		{
			GLTFImporter::Scene scene = Loader::loadGltfScene(resourceManagers, "Sponza/Sponza.gltf", glm::scale(glm::mat4(1.f), glm::vec3(200.f)));
			for (auto& node : scene.mMeshNodes) {
				auto entity = ecs.createEntity();
				if (!node.mName.empty()) {
					ecs.addComponent<TagComponent>(entity, node.mName);
				}
				ecs.addComponent<SpatialComponent>(entity, node.mSpatial);
				ecs.addComponent<MeshComponent>(entity, node.mMeshHandle);
				ecs.addComponent<BoundingBoxComponent>(entity, node.mMin, node.mMax, true);
				if (node.mAlphaMode == GLTFImporter::MeshNode::AlphaMode::Opaque) {
					ecs.addComponent<OpaqueComponent>(entity);
				}
				else if (node.mAlphaMode == GLTFImporter::MeshNode::AlphaMode::AlphaTest) {
					ecs.addComponent<AlphaTestComponent>(entity);
				}
				ecs.addComponent<MaterialComponent>(entity, node.mMaterial);

				ecs.addComponent<ShadowCasterRenderComponent>(entity);
				ecs.addComponent<PBRDeferredComponent>(entity);
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
		ImGui::Checkbox("IBL", &mDrawIBL);
		ImGui::Checkbox("Tonemap", &mDoTonemap);
		ImGui::Checkbox("Bloom", &mDoBloom);
		if (mDoBloom) {
			ImGui::SliderFloat("Bloom Radius", &mBloomRadius, 0.f, 0.01f);
		}
	}

	void Demo::render(const ResourceManagers& resourceManagers, const ECS& ecs, Framebuffer& backbuffer) {
		convolveCubemap(resourceManagers, ecs);

		const auto& cameraTuple = ecs.getSingleView<MainCameraComponent, CameraComponent, SpatialComponent>();
		if (!cameraTuple) {
			return;
		}
		const auto& [cameraEntity, _, camera, cameraSpatial] = *cameraTuple;

		const auto viewport = std::get<1>(*ecs.cGetComponent<ViewportDetailsComponent>());

		TextureHandle shadowTexture = NEO_INVALID_HANDLE;
		if (mDrawShadows) {
			shadowTexture = resourceManagers.mTextureManager.asyncLoad("Shadow map",
				TextureBuilder{}
					.setDimension(glm::u16vec3(2048, 2048, 0))
					.setFormat(TextureFormat{types::texture::Target::Texture2D, types::texture::InternalFormats::D16})
			);
			FramebufferHandle shadowTarget = resourceManagers.mFramebufferManager.asyncLoad(
				"Shadow map",
				FramebufferExternalHandles{ shadowTexture },
				resourceManagers.mTextureManager
			);

			if (resourceManagers.mFramebufferManager.isValid(shadowTarget)) {
				auto& shadowMap = resourceManagers.mFramebufferManager.resolve(shadowTarget);
				shadowMap.bind();
				shadowMap.clear(glm::uvec4(0.f, 0.f, 0.f, 0.f), types::framebuffer::AttachmentBit::Depth);
				drawShadows<OpaqueComponent>(resourceManagers, shadowMap, ecs);
				drawShadows<AlphaTestComponent>(resourceManagers, shadowMap, ecs);
			}
		}

		auto gbufferHandle = createGbuffer(resourceManagers, viewport.mSize);
		if (!resourceManagers.mFramebufferManager.isValid(gbufferHandle)) {
			return;
		}
		auto gbuffer = resourceManagers.mFramebufferManager.resolve(gbufferHandle);
		gbuffer.bind();
		gbuffer.clear(glm::vec4(0.f), types::framebuffer::AttachmentBit::Color | types::framebuffer::AttachmentBit::Depth);
		glViewport(0, 0, viewport.mSize.x, viewport.mSize.y);
		drawGBuffer<OpaqueComponent>(resourceManagers, ecs, cameraEntity, gbufferHandle);
		drawGBuffer<AlphaTestComponent>(resourceManagers, ecs, cameraEntity, gbufferHandle);

		NEO_UNUSED(backbuffer);

		auto hdrColorOutput = resourceManagers.mFramebufferManager.asyncLoad(
			"HDR Color",
			FramebufferBuilder{}
				.setSize(viewport.mSize)
				.attach(TextureFormat{ types::texture::Target::Texture2D, types::texture::InternalFormats::RGB16_F }),
			resourceManagers.mTextureManager
		);
		if (!resourceManagers.mFramebufferManager.isValid(hdrColorOutput)) {
			return;
		}
		auto& hdrColor = resourceManagers.mFramebufferManager.resolve(hdrColorOutput);

		hdrColor.bind();
		hdrColor.clear(glm::vec4(0.f, 0.f, 0.f, 1.f), types::framebuffer::AttachmentBit::Color);
		glViewport(0, 0, viewport.mSize.x, viewport.mSize.y);
		drawSkybox(resourceManagers, ecs, cameraEntity);
		drawDirectionalLightResolve(resourceManagers, ecs, cameraEntity, gbufferHandle, shadowTexture);

		/*
		// Extract IBL
		std::optional<IBLComponent> ibl;
		const auto iblTuple = ecs.getSingleView<SkyboxComponent, IBLComponent>();
		if (iblTuple && mDrawIBL) {
			const auto& _ibl = std::get<2>(*iblTuple);
			if (_ibl.mConvolved && _ibl.mDFGGenerated) {
				ibl = _ibl;
			}
		}

		drawPBR<OpaqueComponent>(resourceManagers, ecs, cameraEntity, shadowTexture, ibl, mDebugMode);
		drawPBR<AlphaTestComponent>(resourceManagers, ecs, cameraEntity, shadowTexture, ibl, mDebugMode);
		*/

		FramebufferHandle bloomHandle = mDoBloom ? bloom(resourceManagers, viewport.mSize, hdrColor.mTextures[0], mBloomRadius, 8) : hdrColorOutput;
		if (mDoBloom && !resourceManagers.mFramebufferManager.isValid(bloomHandle)) {
			bloomHandle = hdrColorOutput;
		}

		FramebufferHandle tonemappedHandle = mDoTonemap ? tonemap(resourceManagers, viewport.mSize, resourceManagers.mFramebufferManager.resolve(bloomHandle).mTextures[0]) : bloomHandle;
		if (mDoTonemap && !resourceManagers.mFramebufferManager.isValid(tonemappedHandle)) {
			tonemappedHandle = bloomHandle;
		}

		backbuffer.bind();
		backbuffer.clear(glm::vec4(0.f, 0.f, 0.f, 1.f), types::framebuffer::AttachmentBit::Color | types::framebuffer::AttachmentBit::Depth);
		drawFXAA(resourceManagers, viewport.mSize, resourceManagers.mFramebufferManager.resolve(tonemappedHandle).mTextures[0]);
		// Don't forget the depth. Because reasons.
		glBlitNamedFramebuffer(gbuffer.mFBOID, backbuffer.mFBOID,
			0, 0, viewport.mSize.x, viewport.mSize.y,
			0, 0, viewport.mSize.x, viewport.mSize.y,
			GL_DEPTH_BUFFER_BIT,
			GL_NEAREST
		);
	}

	void Demo::destroy() {
	}
}
