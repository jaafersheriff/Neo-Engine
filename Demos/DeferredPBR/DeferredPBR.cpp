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

		// _createPointLights(ecs, resourceManagers, 2);

		// Dialectric spheres
		static float numSpheres = 8;
		for (int i = 0; i < numSpheres; i++) {
			MaterialComponent material;
			material.mAlbedoColor = glm::vec4(1, 0, 0, 1);
			material.mMetallic = 0.f;
			material.mRoughness = 1.f - i / (numSpheres - 1);

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
			material.mRoughness = 1.f - i / (numSpheres - 1);

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

		Loader::loadGltfScene(ecs, resourceManagers, "DamagedHelmet.glb", glm::translate(glm::mat4(1.f), glm::vec3(0.f, 2.5f, -0.5f)),
			[](ECS& ecs, const GLTFImporter::MeshNode& node) {
				ECS::EntityBuilder builder;
				if (!node.mName.empty()) {
					builder.attachComponent<TagComponent>(node.mName);
				}
				builder.attachComponent<SpatialComponent>(node.mSpatial);
				builder.attachComponent<MeshComponent>(node.mMeshHandle);
				builder.attachComponent<BoundingBoxComponent>(node.mMin, node.mMax);
				if (node.mAlphaMode == GLTFImporter::MeshNode::AlphaMode::Opaque) {
					builder.attachComponent<OpaqueComponent>();
				}
				else if (node.mAlphaMode == GLTFImporter::MeshNode::AlphaMode::AlphaTest) {
					builder.attachComponent<AlphaTestComponent>();
				}
				// The emissive factor is 1.0 for some reason
				MaterialComponent material = node.mMaterial;
				material.mEmissiveFactor = glm::vec3(100.f);
				builder.attachComponent<MaterialComponent>(material);
				builder.attachComponent<RotationComponent>(glm::vec3(0.f, 0.5f, 0.f));
				builder.attachComponent<ShadowCasterRenderComponent>();
				builder.attachComponent<PinnedComponent>();
				builder.attachComponent<DeferredPBRRenderComponent>();

				ecs.submitEntity(std::move(builder));
			});
		Loader::loadGltfScene(ecs, resourceManagers, "fblock.gltf", glm::scale(glm::translate(glm::mat4(1.f), glm::vec3(-5.f, 2.5f, -0.5f)), glm::vec3(2.f)),
			[](ECS& ecs, const GLTFImporter::MeshNode& node) {
				SpatialComponent spatial = node.mSpatial;
				spatial.setLookDir(glm::vec3(0.f, 0.4f, 0.1f));
				ecs.submitEntity(std::move(ECS::EntityBuilder{}
					.attachComponent<TagComponent>("Bust")
					.attachComponent<MeshComponent>(node.mMeshHandle)
					.attachComponent<BoundingBoxComponent>(node.mMin, node.mMax)
					.attachComponent<OpaqueComponent>()
					.attachComponent<SpatialComponent>(spatial)
					.attachComponent<MaterialComponent>(node.mMaterial)
					.attachComponent<RotationComponent>(glm::vec3(0.f, 0.5f, 0.f))
					.attachComponent<ShadowCasterRenderComponent>()
					.attachComponent<DeferredPBRRenderComponent>()
				));
			});
		Loader::loadGltfScene(ecs, resourceManagers, "Sponza/Sponza.gltf", glm::scale(glm::mat4(1.f), glm::vec3(200.f)),
			[](ECS& ecs, const GLTFImporter::MeshNode& node) {
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
			});
		Loader::loadGltfScene(ecs, resourceManagers, "porsche/scene.gltf", glm::rotate(glm::translate(glm::mat4(1.f), glm::vec3(-6.75f, 0., -0.25f)), util::PI / 2.f, glm::vec3(0, 1, 0)),
			[](ECS& ecs, const GLTFImporter::MeshNode& node) {
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
			});

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

	void Demo::render(const ResourceManagers& resourceManagers, const ECS& ecs, Framebuffer& backbuffer) {
		convolveCubemap(resourceManagers, ecs);

		const auto& cameraTuple = ecs.getSingleView<MainCameraComponent, CameraComponent, SpatialComponent>();
		if (!cameraTuple) {
			return;
		}
		const auto& [cameraEntity, _, camera, cameraSpatial] = *cameraTuple;

		const auto viewport = std::get<1>(*ecs.cGetComponent<ViewportDetailsComponent>());

		auto lightView = ecs.getSingleView<MainLightComponent, DirectionalLightComponent, ShadowCameraComponent>();
		if (lightView) {
			auto& [lightEntity, __, ___, shadowCamera] = *lightView;
			if (mDrawDirectionalShadows) {
				if (resourceManagers.mTextureManager.isValid(shadowCamera.mShadowMap)) {
					auto& shadowTexture = resourceManagers.mTextureManager.resolve(shadowCamera.mShadowMap);
					glViewport(0, 0, shadowTexture.mWidth, shadowTexture.mHeight);
					drawShadows<OpaqueComponent>(resourceManagers, ecs, lightEntity, true);
					drawShadows<AlphaTestComponent>(resourceManagers, ecs, lightEntity, false);
					//drawShadows<TransparentComponent>(resourceManagers, ecs, lightEntity, false);
				}
			}
		}

		auto pointLightView = ecs.getView<PointLightComponent, ShadowCameraComponent, SpatialComponent>();
		for (auto& entity : pointLightView) {
			const auto& shadowCamera = pointLightView.get<ShadowCameraComponent>(entity);
			if (mDrawPointLightShadows) {
				if (resourceManagers.mTextureManager.isValid(shadowCamera.mShadowMap)) {
					auto& shadowTexture = resourceManagers.mTextureManager.resolve(shadowCamera.mShadowMap);
					glViewport(0, 0, shadowTexture.mWidth, shadowTexture.mHeight);
					drawPointLightShadows<OpaqueComponent>(resourceManagers, ecs, entity, true);
					drawPointLightShadows<AlphaTestComponent>(resourceManagers, ecs, entity, false);
					//drawPointLightShadows<TransparentComponent>(resourceManagers, ecs, entity, false);
				}
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

		if (mGbufferDebugParams.mDebugMode != GBufferDebugParameters::DebugMode::Off) {
			auto debugOutput = drawGBufferDebug(resourceManagers, gbufferHandle, viewport.mSize, mGbufferDebugParams);
			if (resourceManagers.mFramebufferManager.isValid(debugOutput)) {
				blit(resourceManagers, backbuffer, resourceManagers.mFramebufferManager.resolve(debugOutput).mTextures[0], viewport.mSize);
				return;
			}
		}

		auto hdrColorOutput = resourceManagers.mFramebufferManager.asyncLoad(
			"HDR Color",
			FramebufferBuilder{}
				.setSize(viewport.mSize)
				.attach(TextureFormat{ types::texture::Target::Texture2D, types::texture::InternalFormats::RGBA16_F })
				.attach(TextureFormat{types::texture::Target::Texture2D, types::texture::InternalFormats::D16}),
			resourceManagers.mTextureManager
		);
		if (!resourceManagers.mFramebufferManager.isValid(hdrColorOutput)) {
			return;
		}
		auto& hdrColor = resourceManagers.mFramebufferManager.resolve(hdrColorOutput);

		{
			TRACY_GPUN("GBuffer Depth Blit");
			hdrColor.bind();
			hdrColor.clear(glm::vec4(0.f, 0.f, 0.f, 1.f), types::framebuffer::AttachmentBit::Color | types::framebuffer::AttachmentBit::Depth);
			glViewport(0, 0, viewport.mSize.x, viewport.mSize.y);
			glBlitNamedFramebuffer(gbuffer.mFBOID, hdrColor.mFBOID,
				0, 0, viewport.mSize.x, viewport.mSize.y,
				0, 0, viewport.mSize.x, viewport.mSize.y,
				GL_DEPTH_BUFFER_BIT,
				GL_NEAREST
			);
		}
		drawDirectionalLightResolve<MainLightComponent>(resourceManagers, ecs, cameraEntity, gbufferHandle);
		drawPointLightResolve(resourceManagers, ecs, cameraEntity, gbufferHandle, viewport.mSize, mLightDebugRadius);
		// Extract IBL
		std::optional<IBLComponent> ibl;
		const auto iblTuple = ecs.getSingleView<SkyboxComponent, IBLComponent>();
		if (iblTuple && mDrawIBL) {
			const auto& _ibl = std::get<2>(*iblTuple);
			if (_ibl.mConvolved && _ibl.mDFGGenerated) {
				ibl = _ibl;
			}
		}
		drawIndirectResolve(resourceManagers, ecs, cameraEntity, gbufferHandle, ibl);
		drawForwardPBR<TransparentComponent>(resourceManagers, ecs, cameraEntity, ibl);
		drawSkybox(resourceManagers, ecs, cameraEntity);

		FramebufferHandle bloomHandle = mDoBloom ? bloom(resourceManagers, viewport.mSize, hdrColor.mTextures[0], mBloomParams) : hdrColorOutput;
		if (mDoBloom && !resourceManagers.mFramebufferManager.isValid(bloomHandle)) {
			bloomHandle = hdrColorOutput;
		}

		TextureHandle averageLuminance = NEO_INVALID_HANDLE;
		{
			auto previousHDRColorHandle = resourceManagers.mFramebufferManager.asyncLoad(
				"Previous HDR Color",
				FramebufferBuilder{}
				.setSize(viewport.mSize)
				.attach(TextureFormat{ types::texture::Target::Texture2D, types::texture::InternalFormats::RGBA16_F }),
				resourceManagers.mTextureManager
			);
			if (resourceManagers.mFramebufferManager.isValid(previousHDRColorHandle)) {
				const auto& previousHDRColor = resourceManagers.mFramebufferManager.resolve(previousHDRColorHandle);
				averageLuminance = calculateAutoexposure(resourceManagers, ecs, previousHDRColor.mTextures[0], mAutoExposureParams);
				TRACY_GPUN("Blit Previous HDR Color");
				blit(resourceManagers, previousHDRColor, resourceManagers.mFramebufferManager.resolve(bloomHandle).mTextures[0], viewport.mSize);
			}
		}

		FramebufferHandle tonemappedHandle = mDoTonemap ? tonemap(resourceManagers, viewport.mSize, resourceManagers.mFramebufferManager.resolve(bloomHandle).mTextures[0], averageLuminance) : bloomHandle;
		if (mDoTonemap && !resourceManagers.mFramebufferManager.isValid(tonemappedHandle)) {
			tonemappedHandle = bloomHandle;
		}

		backbuffer.bind();
		backbuffer.clear(glm::vec4(0.f, 0.f, 0.f, 1.f), types::framebuffer::AttachmentBit::Color | types::framebuffer::AttachmentBit::Depth);
		drawFXAA(resourceManagers, viewport.mSize, resourceManagers.mFramebufferManager.resolve(tonemappedHandle).mTextures[0]);
		// Don't forget the depth. Because reasons.
		glBlitNamedFramebuffer(hdrColor.mFBOID, backbuffer.mFBOID,
			0, 0, viewport.mSize.x, viewport.mSize.y,
			0, 0, viewport.mSize.x, viewport.mSize.y,
			GL_DEPTH_BUFFER_BIT,
			GL_NEAREST
		);
	}

	void Demo::destroy() {
	}
}
