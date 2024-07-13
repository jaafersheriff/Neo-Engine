#include "FireworkDemo.hpp"
#include "Engine/Engine.hpp"

#include "ECS/ECS.hpp"
#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/CameraComponent/CameraControllerComponent.hpp"
#include "ECS/Component/CameraComponent/MainCameraComponent.hpp"
#include "ECS/Component/CollisionComponent/BoundingBoxComponent.hpp"
#include "ECS/Component/EngineComponents/TagComponent.hpp"
#include "ECS/Component/EngineComponents/FrameStatsComponent.hpp"
#include "ECS/Component/HardwareComponent/ViewportDetailsComponent.hpp"
#include "ECS/Component/LightComponent/LightComponent.hpp"
#include "ECS/Component/RenderingComponent/MeshComponent.hpp"
#include "ECS/Component/RenderingComponent/MaterialComponent.hpp"
#include "ECS/Component/RenderingComponent/ForwardPBRRenderComponent.hpp"
#include "ECS/Component/RenderingComponent/WireframeRenderComponent.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"
#include "ECS/Component/SpatialComponent/RotationComponent.hpp"

#include "ECS/Systems/CameraSystems/CameraControllerSystem.hpp"
#include "ECS/Systems/TranslationSystems/RotationSystem.hpp"

#include "Renderer/RenderingSystems/ForwardPBRRenderer.hpp"
#include "Renderer/RenderingSystems/FXAARenderer.hpp"
#include "Renderer/RenderingSystems/PointLightShadowMapRenderer.hpp"
#include "Renderer/RenderingSystems/WireframeRenderer.hpp"
#include "Renderer/RenderingSystems/TonemapRenderer.hpp"

#include "Loader/GLTFImporter.hpp"

#include "glm/gtc/matrix_transform.hpp"

#include "ECS/Component/RenderingComponent/LineMeshComponent.hpp"

using namespace neo;

namespace Fireworks {

	namespace {
		START_COMPONENT(FireworkComponent);
		FireworkComponent(const MeshManager& meshManager, uint32_t count) 
			: mCount(count)
			, mNeedsInit(true)
		{
			MeshLoadDetails details;
			details.mPrimtive = types::mesh::Primitive::Points;
			details.mVertexBuffers[types::mesh::VertexType::Position] = {
				8,
				0,
				types::ByteFormats::Float,
				false,
				mCount,
				0,
				static_cast<uint32_t>(sizeof(float) * 8 * mCount),
				nullptr
			};
			mBuffer = meshManager.asyncLoad("Particles Buffer", details);
		}

		virtual void imGuiEditor() {
			if (ImGui::Button("Reset")) {
				mNeedsInit = true;
			}
		}

		MeshHandle mBuffer;
		uint32_t mCount;
		bool mNeedsInit;
		END_COMPONENT();

		void _tickParticles(const ResourceManagers& resourceManagers, const ECS& ecs) {
			// Update the mesh
			for(const auto fireworkView : ecs.getView<SpatialComponent, FireworkComponent>().each()) {
				TRACY_GPU();
				const auto& [_, spatial, firework] = fireworkView;

				if (!resourceManagers.mMeshManager.isValid(firework.mBuffer)) {
					continue;
				}

				auto fireworksComputeShaderHandle = resourceManagers.mShaderManager.asyncLoad("FireworksCompute", SourceShader::ConstructionArgs{
					{ types::shader::Stage::Compute, "firework/firework_tick.compute" }
					});
				if (!resourceManagers.mShaderManager.isValid(fireworksComputeShaderHandle)) {
					return;
				}
				ShaderDefines defines;
				MakeDefine(INIT);
				if (firework.mNeedsInit) {
					defines.set(INIT);
					firework.mNeedsInit = false;
				}

				auto& fireworksComputeShader = resourceManagers.mShaderManager.resolveDefines(fireworksComputeShaderHandle, defines);
				fireworksComputeShader.bind();

				float timeStep = 0.f;
				if (auto frameStatsView = ecs.cGetComponent<FrameStatsComponent>()) {
					auto&& [__, frameStats] = *frameStatsView;
					timeStep = frameStats.mDT / 1000.f;
				}
				fireworksComputeShader.bindUniform("timestep", timeStep);
				fireworksComputeShader.bindUniform("lightPos", spatial.getPosition());

				// Bind mesh
				auto& mesh = resourceManagers.mMeshManager.resolve(firework.mBuffer);
				glBindVertexArray(mesh.mVAOID);
				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, mesh.getVBO(types::mesh::VertexType::Position).vboID);
				ShaderBarrier barrier(types::shader::Barrier::StorageBuffer);

				// Dispatch 
				fireworksComputeShader.dispatch({ std::ceil(firework.mCount / 16), 1, 1 });

				// Reset bind
				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
			}
		}

		void _drawParticles(const ResourceManagers& resourceManagers, const ECS& ecs) {
			TRACY_GPU();
			auto fireworksVisShaderHandle = resourceManagers.mShaderManager.asyncLoad("FireworkDraw", SourceShader::ConstructionArgs{
				{ types::shader::Stage::Vertex,   "firework/firework.vert" },
				{ types::shader::Stage::Geometry, "firework/firework.geom" },
				{ types::shader::Stage::Fragment, "firework/firework.frag" },
				});

			if (!resourceManagers.mShaderManager.isValid(fireworksVisShaderHandle)) {
				return;
			}

			auto& fireworksVisShader = resourceManagers.mShaderManager.resolveDefines(fireworksVisShaderHandle, {});
			fireworksVisShader.bind();

			if (auto cameraView = ecs.getSingleView<MainCameraComponent, CameraComponent, SpatialComponent>()) {
				auto&& [_, __, camera, camSpatial] = *cameraView;
				fireworksVisShader.bindUniform("P", camera.getProj());
				fireworksVisShader.bindUniform("V", camSpatial.getView());
			}

			if (auto meshView = ecs.getSingleView<FireworkComponent, SpatialComponent>()) {
				auto&& [_, firework, spatial] = *meshView;
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE);
				glDisable(GL_DEPTH_TEST);
				glDisable(GL_CULL_FACE);

				fireworksVisShader.bindUniform("M", spatial.getModelMatrix());

				/* DRAW */
				resourceManagers.mMeshManager.resolve(firework.mBuffer).draw(firework.mCount);
			}
		}
	}

	IDemo::Config Demo::getConfig() const {
		IDemo::Config config;
		config.name = "Fireworks";
		return config;
	}

	void Demo::init(ECS& ecs, ResourceManagers& resourceManagers) {
		NEO_UNUSED(ecs, resourceManagers);
		{
			auto entity = ecs.createEntity();
			ecs.addComponent<TagComponent>(entity, "Camera");
			ecs.addComponent<SpatialComponent>(entity, glm::vec3(0, 0.6f, 5), glm::vec3(1.f));
			ecs.addComponent<CameraComponent>(entity, 1.f, 100.f, CameraComponent::Perspective{ 45.f, 1.f });
			ecs.addComponent<CameraControllerComponent>(entity, 0.4f, 7.f);
			ecs.addComponent<MainCameraComponent>(entity);
		}

		{
			auto entity = ecs.createEntity();
			ecs.addComponent<TagComponent>(entity, "Light");
			ecs.addComponent<SpatialComponent>(entity, glm::vec3(0.f, 4.f, 7.f), glm::vec3(100.f));
			ecs.addComponent<LightComponent>(entity, glm::vec3(1.f), 1000.f);
			ecs.addComponent<MainLightComponent>(entity);
			ecs.addComponent<PointLightComponent>(entity);
			ecs.addComponent<BoundingBoxComponent>(entity, glm::vec3(-0.5f), glm::vec3(0.5f));
			ecs.addComponent<WireframeRenderComponent>(entity);
			ecs.addComponent<MeshComponent>(entity, HashedString("sphere"));
			ecs.addComponent<ShadowCameraComponent>(entity, entity, types::texture::Target::TextureCube, 512, resourceManagers.mTextureManager);
			ecs.addComponent<FireworkComponent>(entity, resourceManagers.mMeshManager, 1000);
		}

		{
			GLTFImporter::Scene gltfScene = Loader::loadGltfScene(resourceManagers, "bunny.gltf");
			auto bunny = ecs.createEntity();
			ecs.addComponent<TagComponent>(bunny, "Bunny");
			ecs.addComponent<SpatialComponent>(bunny, glm::vec3(2.f, 0.0f, -1.f), glm::vec3(1.5f));
			ecs.addComponent<RotationComponent>(bunny, glm::vec3(0.f, 1.0f, 0.f));
			ecs.addComponent<MeshComponent>(bunny, gltfScene.mMeshNodes[0].mMeshHandle);
			ecs.addComponent<BoundingBoxComponent>(bunny, gltfScene.mMeshNodes[0].mMin, gltfScene.mMeshNodes[0].mMax);
			ecs.addComponent<ForwardPBRRenderComponent>(bunny);
			ecs.addComponent<OpaqueComponent>(bunny);
			ecs.addComponent<ShadowCasterRenderComponent>(bunny);
			auto material = ecs.addComponent<MaterialComponent>(bunny);
			material->mAlbedoColor = glm::vec4(1.f, 0.f, 1.f, 1.f);
		}
		{
			auto icosahedron = ecs.createEntity();
			ecs.addComponent<TagComponent>(icosahedron, "Icosahedron");
			ecs.addComponent<SpatialComponent>(icosahedron, glm::vec3(-2.f, 1.0f, -1.f), glm::vec3(1.5f));
			ecs.addComponent<RotationComponent>(icosahedron, glm::vec3(1.f, 0.0f, 0.f));
			ecs.addComponent<MeshComponent>(icosahedron, HashedString("icosahedron"));
			ecs.addComponent<BoundingBoxComponent>(icosahedron, glm::vec3(-0.5f), glm::vec3(0.5f));
			ecs.addComponent<ForwardPBRRenderComponent>(icosahedron);
			ecs.addComponent<OpaqueComponent>(icosahedron);
			ecs.addComponent<ShadowCasterRenderComponent>(icosahedron);
			auto material = ecs.addComponent<MaterialComponent>(icosahedron);
			material->mAlbedoColor = glm::vec4(1.f, 1.f, 0.f, 1.f);
		}
		{
			auto plane = ecs.createEntity();
			ecs.addComponent<TagComponent>(plane, "Grid");
			ecs.addComponent<SpatialComponent>(plane, glm::vec3(0.f), glm::vec3(15.f, 15.f, 1.f), glm::vec3(-util::PI / 2.f, 0.f, 0.f));
			ecs.addComponent<MeshComponent>(plane, HashedString("quad"));
			ecs.addComponent<BoundingBoxComponent>(plane, glm::vec3(-0.5f, -0.5f, -0.01f), glm::vec3(0.5f, 0.5f, 0.01f), true);
			ecs.addComponent<ForwardPBRRenderComponent>(plane);
			ecs.addComponent<OpaqueComponent>(plane);
			ecs.addComponent<ShadowCasterRenderComponent>(plane);
			auto material = ecs.addComponent<MaterialComponent>(plane);
			material->mAlbedoColor = glm::vec4(1.f);
		}

		ecs.addSystem<CameraControllerSystem>();
		ecs.addSystem<RotationSystem>();
	}

	void Demo::imGuiEditor(ECS& ecs, ResourceManagers& resourceManagers) {
		NEO_UNUSED(ecs, resourceManagers);
	}

	void Demo::update(ECS& ecs, ResourceManagers& resourceManagers) {
		NEO_UNUSED(ecs, resourceManagers);
	}

	void Demo::render(const ResourceManagers& resourceManagers, const ECS& ecs, Framebuffer& backbuffer) {
		_tickParticles(resourceManagers, ecs);

		if (const auto& lightView = ecs.getSingleView<MainLightComponent, PointLightComponent, ShadowCameraComponent>()) {
			const auto& [lightEntity, _, __, shadowCamera] = *lightView;
			if (resourceManagers.mTextureManager.isValid(shadowCamera.mShadowMap)) {
				auto& shadowTexture = resourceManagers.mTextureManager.resolve(shadowCamera.mShadowMap);
				glViewport(0, 0, shadowTexture.mWidth, shadowTexture.mHeight);
				drawPointLightShadows<OpaqueComponent>(resourceManagers, ecs, lightEntity, true);
			}
		}

		const auto&& [cameraEntity, _, cameraSpatial] = *ecs.getSingleView<MainCameraComponent, SpatialComponent>();
		auto viewport = std::get<1>(*ecs.cGetComponent<ViewportDetailsComponent>());

		auto sceneTargetHandle = resourceManagers.mFramebufferManager.asyncLoad(
			"Scene Target",
			FramebufferBuilder{}
			.setSize(viewport.mSize)
			.attach(TextureFormat{ types::texture::Target::Texture2D, types::texture::InternalFormats::RGB16_UNORM })
			.attach(TextureFormat{ types::texture::Target::Texture2D,types::texture::InternalFormats::D16 }),
			resourceManagers.mTextureManager
		);
		if (!resourceManagers.mFramebufferManager.isValid(sceneTargetHandle)) {
			return;
		}
		auto& sceneTarget = resourceManagers.mFramebufferManager.resolve(sceneTargetHandle);
		sceneTarget.bind();
		sceneTarget.clear(glm::vec4(0.f, 0.f, 0.f, 1.f), types::framebuffer::AttachmentBit::Color | types::framebuffer::AttachmentBit::Depth);
		glViewport(0, 0, viewport.mSize.x, viewport.mSize.y);

		drawForwardPBR<OpaqueComponent>(resourceManagers, ecs, cameraEntity);
		_drawParticles(resourceManagers, ecs);
		drawWireframe<LightComponent>(resourceManagers, ecs, cameraEntity);

		FramebufferHandle tonemappedHandle = tonemap(resourceManagers, viewport.mSize, resourceManagers.mFramebufferManager.resolve(sceneTargetHandle).mTextures[0]);
		if (!resourceManagers.mFramebufferManager.isValid(tonemappedHandle)) {
			return;
		}

		backbuffer.bind();
		backbuffer.clear(glm::vec4(0.f, 0.f, 0.f, 1.f), types::framebuffer::AttachmentBit::Color);
		drawFXAA(resourceManagers, viewport.mSize, resourceManagers.mFramebufferManager.resolve(tonemappedHandle).mTextures[0]);
		// Don't forget the depth. Because reasons.
		glBlitNamedFramebuffer(sceneTarget.mFBOID, backbuffer.mFBOID,
			0, 0, viewport.mSize.x, viewport.mSize.y,
			0, 0, viewport.mSize.x, viewport.mSize.y,
			GL_DEPTH_BUFFER_BIT,
			GL_NEAREST
		);
	}

	void Demo::destroy() {
	}
}
