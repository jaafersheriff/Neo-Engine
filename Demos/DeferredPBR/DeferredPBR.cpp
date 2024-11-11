#include "DeferredPBR/DeferredPBR.hpp"
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
#include "ECS/Component/SpatialComponent/SinTranslateComponent.hpp"
#include "ECS/Component/SpatialComponent/RotationComponent.hpp"

#include "ECS/Systems/CameraSystems/CameraControllerSystem.hpp"
#include "ECS/Systems/CameraSystems/FrustaFittingSystem.hpp"
#include "ECS/Systems/TranslationSystems/RotationSystem.hpp"
#include "ECS/Systems/TranslationSystems/SinTranslateSystem.hpp"

#include "DeferredPBRRenderer.hpp"

#include "Renderer/RenderingSystems/Blitter.hpp"
#include "Renderer/RenderingSystems/ConvolveRenderer.hpp"
#include "Renderer/RenderingSystems/ForwardPBRRenderer.hpp"
#include "Renderer/RenderingSystems/FXAARenderer.hpp"
#include "Renderer/RenderingSystems/ShadowMapRenderer.hpp"
#include "Renderer/RenderingSystems/SkyboxRenderer.hpp"
#include "Renderer/RenderingSystems/TonemapRenderer.hpp"
#include "Renderer/GLObjects/Framebuffer.hpp"
#include "Renderer/GLObjects/ResolvedShaderInstance.hpp"

#include "Loader/GLTFImporter.hpp"

#include "glm/gtc/matrix_transform.hpp"

using namespace neo;

namespace DeferredPBR {
	namespace {
		void _createPointLights(ECS& ecs, ResourceManagers& resourceManagers, const int count) {
			for (auto& e : ecs.getView<PointLightComponent>()) {
				if (ecs.has<ShadowCameraComponent>(e)) {
					resourceManagers.mTextureManager.discard(ecs.getComponent<ShadowCameraComponent>(e)->mShadowMap);
				}
				ecs.removeEntity(e);
			}

			for (int i = 0; i < count; i++) {
				glm::vec3 position(
					util::genRandom(-15.f, 15.f),
					util::genRandom(0.f, 10.f),
					util::genRandom(-7.5f, 7.5f)
				);
				ShadowCameraComponent shadowCamera(types::texture::Target::TextureCube, 256, resourceManagers.mTextureManager);
				ecs.submitEntity(std::move(ECS::EntityBuilder{}
					.attachComponent<LightComponent>(util::genRandomVec3(0.3f, 1.f), util::genRandom(300.f, 1000.f))
					.attachComponent<PointLightComponent>()
					.attachComponent<SinTranslateComponent>(glm::vec3(0.f, util::genRandom(0.f, 5.f), 0.f), position)
					.attachComponent<SpatialComponent>(position, glm::vec3(50.f))
					.attachComponent<BoundingBoxComponent>(glm::vec3(-0.5f), glm::vec3(0.5f), false)
					.attachComponent<ShadowCameraComponent>(shadowCamera)
				));
			}
		}
	}

	IDemo::Config Demo::getConfig() const {
		IDemo::Config config;
		config.name = "DeferredPBR";
		return config;
	}

	void Demo::init(ECS& ecs, ResourceManagers& resourceManagers) {

		{
			ecs.submitEntity(std::move(ECS::EntityBuilder{}
				.attachComponent<CameraControllerComponent>(0.4f, 15.f)
				.attachComponent<MainCameraComponent>()
				.attachComponent<FrustumComponent>()
				.attachComponent<FrustumFitSourceComponent>()
				.attachComponent<PinnedComponent>()
				.attachComponent<TagComponent>("Camera")
				.attachComponent<SpatialComponent>(glm::vec3(0.05f, 0.03f, 0.0f), glm::vec3(1.f))
				.attachComponent<CameraComponent>(1.f, 35.f, CameraComponent::Perspective{ 45.f, 1.f })
			));
		}
		{
			SpatialComponent spatial(glm::vec3(75.f, 200.f, 20.f));
			spatial.setLookDir(glm::normalize(glm::vec3(-0.28f, -0.96f, -0.06f)));
			ShadowCameraComponent shadowCamera(types::texture::Target::Texture2D, 2048, resourceManagers.mTextureManager);

			ecs.submitEntity(std::move(ECS::EntityBuilder{}
				.attachComponent<TagComponent>("Light")
				.attachComponent<SpatialComponent>(spatial)
				.attachComponent<LightComponent>(glm::vec3(0.978f, 0.903f, 0.714f), 3000.f)
				.attachComponent<MainLightComponent>()
				.attachComponent<DirectionalLightComponent>()
				.attachComponent<PinnedComponent>()
				.attachComponent<CameraComponent>(-1.f, 1000.f, CameraComponent::Orthographic{ glm::vec2(-100.f, 100.f), glm::vec2(-100.f, 100.f) })
				.attachComponent<ShadowCameraComponent>(shadowCamera)
				.attachComponent<FrustumComponent>()
				.attachComponent<FrustumFitReceiverComponent>(1.f)
			));
		}
		_createPointLights(ecs, resourceManagers, 2);

		// Dialectric spheres
		static float numSpheres = 8;
		for (int i = 0; i < numSpheres; i++) {
			MaterialComponent material;
			material.mAlbedoColor = glm::vec4(1, 0, 0, 1);
			material.mMetallic = 0.f;
			material.mRoughness = 1.f - i / (numSpheres-1);

			ecs.submitEntity(std::move(ECS::EntityBuilder{}
				.attachComponent<SpatialComponent>(glm::vec3(-2.f + i, 1.f, 0.f), glm::vec3(0.6f))
				.attachComponent<MeshComponent>(HashedString("sphere"))
				.attachComponent<BoundingBoxComponent>(glm::vec3(-0.5f), glm::vec3(0.5f))
				.attachComponent<OpaqueComponent>()
				.attachComponent<MaterialComponent>(material)
				.attachComponent<ShadowCasterRenderComponent>()
				.attachComponent<DeferredPBRRenderComponent>()
			));
		}
		// Conductive spheres
		for (int i = 0; i < numSpheres; i++) {
			MaterialComponent material;
			material.mAlbedoColor = glm::vec4(0.944f, 0.776f, 0.373f, 1);
			material.mMetallic = 1.f;
			material.mRoughness = 1.f - i / (numSpheres-1);

			ecs.submitEntity(std::move(ECS::EntityBuilder{}
				.attachComponent<SpatialComponent>(glm::vec3(-2.f + i, 1.f, -1.5f), glm::vec3(0.6f))
				.attachComponent<MeshComponent>(HashedString("sphere"))
				.attachComponent<BoundingBoxComponent>(glm::vec3(-0.5f), glm::vec3(0.5f))
				.attachComponent<OpaqueComponent>()
				.attachComponent<MaterialComponent>(material)
				.attachComponent<ShadowCasterRenderComponent>()
				.attachComponent<DeferredPBRRenderComponent>()
			));
		}
		{
			MaterialComponent material;
			material.mAlbedoColor = glm::vec4(0.25f, 0.f, 1.f, 1);
			material.mMetallic = 0.f;
			material.mRoughness = 0.15f;

			ecs.submitEntity(std::move(ECS::EntityBuilder{}
				.attachComponent<TagComponent>("Icosahedron")
				.attachComponent<SpatialComponent>(glm::vec3(-3.f, 4.0f, -0.5f), glm::vec3(1.f))
				.attachComponent<RotationComponent>(glm::vec3(0.f, 0.0f, 1.f))
				.attachComponent<MeshComponent>(HashedString("icosahedron"))
				.attachComponent<BoundingBoxComponent>(glm::vec3(-0.5f), glm::vec3(0.5f))
				.attachComponent<OpaqueComponent>()
				.attachComponent<MaterialComponent>(material)
				.attachComponent<ShadowCasterRenderComponent>()
				.attachComponent<PinnedComponent>()
				.attachComponent<DeferredPBRRenderComponent>()
			));
		}

		// Emissive sphere
		{
			MaterialComponent material;
			material.mAlbedoColor = glm::vec4(1.f);
			material.mMetallic = 0.f;
			material.mRoughness = 0.f;
			material.mEmissiveFactor = glm::vec3(10000.f);

			ecs.submitEntity(std::move(ECS::EntityBuilder{}
				.attachComponent<SpatialComponent>(glm::vec3(0.f, 1.f, -0.75f), glm::vec3(0.6f))
				.attachComponent<MeshComponent>(HashedString("sphere"))
				.attachComponent<BoundingBoxComponent>(glm::vec3(-0.5f), glm::vec3(0.5f))
				.attachComponent<OpaqueComponent>()
				.attachComponent<MaterialComponent>(material)
				.attachComponent<ShadowCasterRenderComponent>()
				.attachComponent<DeferredPBRRenderComponent>()
			));
		}

		{
			ecs.submitEntity(std::move(ECS::EntityBuilder{}
				.attachComponent<TagComponent>("Skybox")
				.attachComponent<SkyboxComponent>(resourceManagers.mTextureManager.asyncLoad("hdr skybox", TextureFiles{
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
					}))
				.attachComponent<IBLComponent>()
				.attachComponent<PinnedComponent>()
			));
		}

		{
			GLTFImporter::MeshNode helmet = Loader::loadGltfScene(resourceManagers, "DamagedHelmet.glb", glm::translate(glm::mat4(1.f), glm::vec3(0.f, 2.5f, -0.5f))).mMeshNodes[0];
			ECS::EntityBuilder builder;
			if (!helmet.mName.empty()) {
				builder.attachComponent<TagComponent>(helmet.mName);
			}
			builder.attachComponent<SpatialComponent>(helmet.mSpatial);
			builder.attachComponent<MeshComponent>(helmet.mMeshHandle);
			builder.attachComponent<BoundingBoxComponent>(helmet.mMin, helmet.mMax);
			if (helmet.mAlphaMode == GLTFImporter::MeshNode::AlphaMode::Opaque) {
				builder.attachComponent<OpaqueComponent>();
			}
			else if (helmet.mAlphaMode == GLTFImporter::MeshNode::AlphaMode::AlphaTest) {
				builder.attachComponent<AlphaTestComponent>();
			}
			// The emissive factor is 1.0 for some reason
			helmet.mMaterial.mEmissiveFactor = glm::vec3(100.f);
			builder.attachComponent<MaterialComponent>(helmet.mMaterial);
			builder.attachComponent<RotationComponent>(glm::vec3(0.f, 0.5f, 0.f));
			builder.attachComponent<ShadowCasterRenderComponent>();
			builder.attachComponent<PinnedComponent>();
			builder.attachComponent<DeferredPBRRenderComponent>();
			ecs.submitEntity(std::move(builder));
		}

		{
			GLTFImporter::MeshNode bust = Loader::loadGltfScene(resourceManagers, "fblock.gltf", glm::scale(glm::translate(glm::mat4(1.f), glm::vec3(-5.f, 2.5f, -0.5f)), glm::vec3(2.f))).mMeshNodes[0];
			SpatialComponent spatial = bust.mSpatial;
			spatial.setLookDir(glm::vec3(0.f, 0.4f, 0.1f));
			ecs.submitEntity(std::move(ECS::EntityBuilder{}
				.attachComponent<TagComponent>("Bust")
				.attachComponent<MeshComponent>(bust.mMeshHandle)
				.attachComponent<BoundingBoxComponent>(bust.mMin, bust.mMax)
				.attachComponent<OpaqueComponent>()
				.attachComponent<SpatialComponent>(spatial)
				.attachComponent<MaterialComponent>(bust.mMaterial)
				.attachComponent<RotationComponent>(glm::vec3(0.f, 0.5f, 0.f))
				.attachComponent<ShadowCasterRenderComponent>()
				.attachComponent<DeferredPBRRenderComponent>()
			));
		}
		{
			GLTFImporter::Scene scene = Loader::loadGltfScene(resourceManagers, "Sponza/Sponza.gltf", glm::scale(glm::mat4(1.f), glm::vec3(200.f)));
			for (auto& node : scene.mMeshNodes) {
				ECS::EntityBuilder builder;
				if (!node.mName.empty()) {
					builder.attachComponent<TagComponent>(node.mName);
				}
				builder.attachComponent<SpatialComponent>(node.mSpatial);
				builder.attachComponent<MeshComponent>(node.mMeshHandle);
				builder.attachComponent<BoundingBoxComponent>(node.mMin, node.mMax, true);
				if (node.mAlphaMode == GLTFImporter::MeshNode::AlphaMode::Transparent) {
					builder.attachComponent<TransparentComponent>();
					builder.attachComponent<ForwardPBRRenderComponent>();
				}
				else if (node.mAlphaMode == GLTFImporter::MeshNode::AlphaMode::AlphaTest) {
					builder.attachComponent<AlphaTestComponent>();
					builder.attachComponent<DeferredPBRRenderComponent>();
				}
				else {
					builder.attachComponent<OpaqueComponent>();
					builder.attachComponent<DeferredPBRRenderComponent>();
				}
				builder.attachComponent<MaterialComponent>(node.mMaterial);
				builder.attachComponent<ShadowCasterRenderComponent>();
				ecs.submitEntity(std::move(builder));
			}
		}
		{
			GLTFImporter::Scene scene = Loader::loadGltfScene(resourceManagers, "porsche/scene.gltf", glm::rotate(glm::translate(glm::mat4(1.f), glm::vec3(-6.75f, 0., -0.25f)), util::PI / 2.f, glm::vec3(0,1,0)));
			for (auto& node : scene.mMeshNodes) {
				ECS::EntityBuilder builder;
				if (!node.mName.empty()) {
					builder.attachComponent<TagComponent>(node.mName);
				}
				builder.attachComponent<SpatialComponent>(node.mSpatial);
				builder.attachComponent<MeshComponent>(node.mMeshHandle);
				builder.attachComponent<BoundingBoxComponent>(node.mMin, node.mMax, true);
				if (node.mAlphaMode == GLTFImporter::MeshNode::AlphaMode::Transparent) {
					builder.attachComponent<TransparentComponent>();
					builder.attachComponent<ForwardPBRRenderComponent>();
				}
				else if (node.mAlphaMode == GLTFImporter::MeshNode::AlphaMode::AlphaTest) {
					builder.attachComponent<AlphaTestComponent>();
					builder.attachComponent<DeferredPBRRenderComponent>();
				}
				else {
					builder.attachComponent<OpaqueComponent>();
					builder.attachComponent<DeferredPBRRenderComponent>();
				}
				builder.attachComponent<MaterialComponent>(node.mMaterial);
				builder.attachComponent<ShadowCasterRenderComponent>();
				ecs.submitEntity(std::move(builder));
			}
		}

		/* Systems - order matters! */
		ecs.addSystem<CameraControllerSystem>();
		ecs.addSystem<RotationSystem>();
		ecs.addSystem<SinTranslateSystem>();
		ecs.addSystem<FrustumSystem>();
		ecs.addSystem<FrustaFittingSystem>();
		ecs.addSystem<FrustumCullingSystem>();
	}

	void Demo::imGuiEditor(ECS& ecs, ResourceManagers& resourceManagers) {
		NEO_UNUSED(ecs);

		mGbufferDebugParams.imguiEditor();

		if (ImGui::Checkbox("Directional Shadows", &mDrawDirectionalShadows)) {
			for (auto& entity : ecs.getView<DirectionalLightComponent, ShadowCameraComponent>()) {
				auto shadowCamera = ecs.getComponent<ShadowCameraComponent>(entity);
				resourceManagers.mTextureManager.discard(shadowCamera->mShadowMap);
				if (mDrawDirectionalShadows) {
					shadowCamera->mShadowMap = resourceManagers.mTextureManager.asyncLoad(
						HashedString(shadowCamera->mID.c_str()),
						TextureBuilder{}
						.setDimension(glm::u16vec3(static_cast<uint16_t>(2048), static_cast<uint16_t>(2048), 0))
						.setFormat(TextureFormat{ types::texture::Target::Texture2D, types::texture::InternalFormats::D16 })
					);
				}
			}
		}
		if (ImGui::Checkbox("Point Light Shadows", &mDrawPointLightShadows)) {
			for (auto& entity : ecs.getView<PointLightComponent, ShadowCameraComponent>()) {
				auto shadowCamera = ecs.getComponent<ShadowCameraComponent>(entity);
				resourceManagers.mTextureManager.discard(shadowCamera->mShadowMap);
				if (mDrawPointLightShadows) {
					shadowCamera->mShadowMap = resourceManagers.mTextureManager.asyncLoad(
						HashedString(shadowCamera->mID.c_str()),
						TextureBuilder{}
						.setDimension(glm::u16vec3(static_cast<uint16_t>(256), static_cast<uint16_t>(256), 0))
						.setFormat(TextureFormat{ types::texture::Target::TextureCube, types::texture::InternalFormats::D16 })
					);
				}
			}
		}
		ImGui::SliderFloat("Debug Radius", &mLightDebugRadius, 0.f, 10.f);
		if (ImGui::SliderInt("# Point Lights", &mPointLightCount, 0, 100)) {
			_createPointLights(ecs, resourceManagers, mPointLightCount);
		}

		ImGui::Checkbox("IBL", &mDrawIBL);
		ImGui::Checkbox("Tonemap", &mDoTonemap);
		if (mDoTonemap) {
			mAutoExposureParams.imguiEditor();
		}
		ImGui::Checkbox("Bloom", &mDoBloom);
		if (mDoBloom) {
			mBloomParams.imguiEditor();
		}
	}

	void Demo::render(FrameGraph& fg, const ResourceManagers& resourceManagers, const ECS& ecs, FramebufferHandle backbufferHandle) {
		NEO_UNUSED(fg, resourceManagers, ecs, backbufferHandle);
		//convolveCubemap(resourceManagers, ecs);

		const auto& cameraTuple = ecs.getSingleView<MainCameraComponent, CameraComponent, SpatialComponent>();
		if (!cameraTuple) {
			return;
		}
		const auto& [cameraEntity, _, camera, cameraSpatial] = *cameraTuple;

		const auto viewport = std::get<1>(*ecs.cGetComponent<ViewportDetailsComponent>());

		auto lightView = ecs.getSingleView<MainLightComponent, DirectionalLightComponent, ShadowCameraComponent>();
		if (lightView) {
			auto& [lightEntity, __, ___, shadowCamera] = *lightView;
			if (mDrawDirectionalShadows && resourceManagers.mTextureManager.isValid(shadowCamera.mShadowMap)) {

				std::string targetName = "ShadowMap_" + std::to_string(static_cast<uint32_t>(lightEntity));
				FramebufferHandle shadowMapHandle = resourceManagers.mFramebufferManager.asyncLoad(
					HashedString(targetName.c_str()),
					FramebufferExternalAttachments{ { shadowCamera.mShadowMap } },
					resourceManagers.mTextureManager
				);

				fg.clear(shadowMapHandle, glm::vec4(0.f), types::framebuffer::AttachmentBit::Depth);
				drawShadows<OpaqueComponent>(fg, shadowMapHandle, resourceManagers, lightEntity);
				drawShadows<AlphaTestComponent>(fg, shadowMapHandle, resourceManagers, lightEntity);
			}
		}

		auto pointLightView = ecs.getView<PointLightComponent, ShadowCameraComponent, SpatialComponent>();
		for (auto& pointLightEntity : pointLightView) {
			const auto& shadowCamera = pointLightView.get<ShadowCameraComponent>(pointLightEntity);
			if (mDrawPointLightShadows && resourceManagers.mTextureManager.isValid(shadowCamera.mShadowMap)) {
				for (uint8_t i = 0; i < 6; i++) {
					char targetName[128];
					sprintf(targetName, "%s_%" PRIu64 "_%d", "PointLightShadowMap", pointLightEntity, i);
					FramebufferHandle shadowTargetHandle = resourceManagers.mFramebufferManager.asyncLoad(
						HashedString(targetName),
						FramebufferExternalAttachments{
							FramebufferAttachment {
								shadowCamera.mShadowMap,
								static_cast<types::framebuffer::AttachmentTarget>(static_cast<uint8_t>(types::framebuffer::AttachmentTarget::TargetCubeX_Positive) + i),
								0
							}
						},
						resourceManagers.mTextureManager
					);
					fg.clear(shadowTargetHandle, glm::vec4(0.f), types::framebuffer::AttachmentBit::Depth);
					drawPointLightShadows<OpaqueComponent>(fg, shadowTargetHandle, resourceManagers, ecs, pointLightEntity, i);
					drawPointLightShadows<AlphaTestComponent>(fg, shadowTargetHandle, resourceManagers, ecs, pointLightEntity, i);
				}
			}
		}

		Viewport sceneViewport(0, 0, viewport.mSize.x, viewport.mSize.y);
		auto gbufferHandle = createGbuffer(resourceManagers, viewport.mSize);
		fg.clear(gbufferHandle, glm::vec4(0.f, 0.f, 0.f, 1.f), types::framebuffer::AttachmentBit::Color | types::framebuffer::AttachmentBit::Depth);
		drawGBuffer<OpaqueComponent>(fg, gbufferHandle, sceneViewport, resourceManagers, cameraEntity);
		drawGBuffer<AlphaTestComponent>(fg, gbufferHandle, sceneViewport, resourceManagers, cameraEntity);

		if (mGbufferDebugParams.mDebugMode != GBufferDebugParameters::DebugMode::Off) {
			auto debugOutput = drawGBufferDebug(fg, gbufferHandle, sceneViewport, resourceManagers, mGbufferDebugParams);
			blit(fg, sceneViewport, resourceManagers, debugOutput, backbufferHandle);
			return;
		}

		// Handle resizing wahoo
		TextureHandle hdrColorHandle("HDR Color");
		if (resourceManagers.mTextureManager.isValid(hdrColorHandle)) {
			const Texture& hdrColor = resourceManagers.mTextureManager.resolve(hdrColorHandle);
			if (viewport.mSize.x != hdrColor.mWidth || viewport.mSize.y != hdrColor.mHeight) {
				resourceManagers.mTextureManager.discard(hdrColorHandle);
			}
		}
		hdrColorHandle = resourceManagers.mTextureManager.asyncLoad(hdrColorHandle, TextureBuilder{}
			.setFormat({ types::texture::Target::Texture2D, types::texture::InternalFormats::RGBA16_F })
			.setDimension(glm::u16vec3(viewport.mSize, 0))
		);
		FramebufferHandle hdrColorTarget;
		if (resourceManagers.mFramebufferManager.isValid(gbufferHandle)) {
			const auto& gbuffer = resourceManagers.mFramebufferManager.resolve(gbufferHandle);
			hdrColorTarget = resourceManagers.mFramebufferManager.asyncLoad(
				"HDR Color",
				FramebufferExternalAttachments{
					FramebufferAttachment{hdrColorHandle},
					FramebufferAttachment{gbuffer.mTextures[3]},
				},
				resourceManagers.mTextureManager
			);
		}
		else {
			return;
		}

		drawDirectionalLightResolve<MainLightComponent>(fg, hdrColorTarget, sceneViewport, resourceManagers, ecs, cameraEntity, gbufferHandle);
		drawPointLightResolve(fg, hdrColorTarget, sceneViewport, resourceManagers, ecs, cameraEntity, gbufferHandle, mLightDebugRadius);
		// drawIndirectResolve(resourceManagers, ecs, cameraEntity, gbufferHandle, ibl);
		drawForwardPBR<TransparentComponent>(fg, hdrColorTarget, sceneViewport, resourceManagers, ecs, cameraEntity);
		drawSkybox(fg, hdrColorTarget, sceneViewport, resourceManagers, cameraEntity);

		FramebufferHandle bloomHandle = mDoBloom ? bloom(fg, hdrColorTarget, sceneViewport, resourceManagers, mBloomParams) : hdrColorTarget;
		if (!resourceManagers.mFramebufferManager.isValid(bloomHandle)) {
			bloomHandle = hdrColorTarget;
		}

		// TextureHandle averageLuminance = NEO_INVALID_HANDLE;
		// {
		// 	auto previousHDRColorHandle = resourceManagers.mFramebufferManager.asyncLoad(
		// 		"Previous HDR Color",
		// 		FramebufferBuilder{}
		// 		.setSize(viewport.mSize)
		// 		.attach(TextureFormat{ types::texture::Target::Texture2D, types::texture::InternalFormats::RGBA16_F }),
		// 		resourceManagers.mTextureManager
		// 	);
		// 	if (resourceManagers.mFramebufferManager.isValid(previousHDRColorHandle)) {
		// 		const auto& previousHDRColor = resourceManagers.mFramebufferManager.resolve(previousHDRColorHandle);
		// 		averageLuminance = calculateAutoexposure(resourceManagers, ecs, previousHDRColor.mTextures[0], mAutoExposureParams);
		// 		TRACY_GPUN("Blit Previous HDR Color");
		// 		blit(resourceManagers, previousHDRColor, resourceManagers.mFramebufferManager.resolve(bloomHandle).mTextures[0], viewport.mSize);
		// 	}
		// }

		// FramebufferHandle tonemappedHandle = mDoTonemap ? tonemap(resourceManagers, viewport.mSize, resourceManagers.mFramebufferManager.resolve(bloomHandle).mTextures[0], averageLuminance) : bloomHandle;
		// if (mDoTonemap && !resourceManagers.mFramebufferManager.isValid(tonemappedHandle)) {
		// 	tonemappedHandle = bloomHandle;
		// }

		fg.clear(backbufferHandle, glm::vec4(0.f, 0.f, 0.f, 1.f), types::framebuffer::AttachmentBit::Color | types::framebuffer::AttachmentBit::Depth);
		// drawFXAA(resourceManagers, viewport.mSize, resourceManagers.mFramebufferManager.resolve(tonemappedHandle).mTextures[0]);
		blit(fg, sceneViewport, resourceManagers, hdrColorTarget, backbufferHandle);
		// // Don't forget the depth. Because reasons.
		// glBlitNamedFramebuffer(hdrColor.mFBOID, backbuffer.mFBOID,
		// 	0, 0, viewport.mSize.x, viewport.mSize.y,
		// 	0, 0, viewport.mSize.x, viewport.mSize.y,
		// 	GL_DEPTH_BUFFER_BIT,
		// 	GL_NEAREST
		// );
	}

	void Demo::destroy() {
	}
}
