#pragma once

#include "Renderer/GLObjects/SourceShader.hpp"
#include "Renderer/GLObjects/ResolvedShaderInstance.hpp"

#include "ECS/Component/RenderingComponent/ImGuiDrawComponent.hpp"

#include "ECS/ECS.hpp"

#include "Util/Util.hpp"

#include <glm/glm.hpp>

namespace neo {

	namespace {
	#pragma warning(push)
	#pragma warning (disable : 4100 )
	template<typename... Deps>
	inline void _drawImGuiMeshes(
		FrameGraph& fg,
		FramebufferHandle outTarget,
		Viewport sceneViewport,
		const ResourceManagers& resourceManagers,
		const ECS& ecs,
		glm::uvec2 viewportOffset,
		glm::uvec2 viewportSize,
		Deps... deps
	) {

		TRACY_ZONE();

		auto shaderHandle = resourceManagers.mShaderManager.asyncLoad("ImGuiShader", SourceShader::ConstructionArgs{
			{ types::shader::Stage::Vertex, "imgui.vert"},
			{ types::shader::Stage::Fragment, "imgui.frag" }
			});

		{
			TRACY_ZONEN("Draw sorting");
			// TODO - sort index should be part of draw key
			ecs.sort<ImGuiComponent, ImGuiDrawComponent>([&ecs](const ECS::Entity entityLeft, const ECS::Entity entityRight) {
				auto leftDraw = ecs.cGetComponent<ImGuiDrawComponent>(entityLeft);
				auto rightDraw = ecs.cGetComponent<ImGuiDrawComponent>(entityRight);
				if (leftDraw && rightDraw) {
					return leftDraw->mDrawOrder < rightDraw->mDrawOrder;
				}
				return false;
				});
		}

		PassState passState;
		passState.mBlending = true;
		passState.mBlendEquation = types::passState::BlendEquation::Add;
		passState.mBlendSrcRGB = types::passState::BlendFactor::Alpha;
		passState.mBlendDstRGB = types::passState::BlendFactor::OneMinusAlpha;
		passState.mBlendSrcAlpha = types::passState::BlendFactor::One;
		passState.mBlendDstAlpha = types::passState::BlendFactor::OneMinusAlpha;
		passState.mCullFace = false;
		passState.mDepthTest = false;
		passState.mStencilTest = false;
		passState.mScissorTest = true;
		float L = static_cast<float>(viewportOffset.x);
		float R = static_cast<float>(viewportOffset.x + viewportSize.x);
		float T = static_cast<float>(viewportOffset.y);
		float B = static_cast<float>(viewportOffset.y + viewportSize.y);
		const glm::mat4 ortho_projection = glm::mat4(
			2.0f / (R - L), 0.0f, 0.0f, 0.0f,
			0.0f, 2.0f / (T - B), 0.0f, 0.0f,
			0.0f, 0.0f, -1.0f, 0.0f,
			(R + L) / (L - R), (T + B) / (B - T), 0.0f, 1.0f
		);

		for (auto&& [_, draw, __] : ecs.getView<ImGuiDrawComponent, ImGuiComponent>().each()) {

			// Need an individual pass per draw b/c scissor..........
			Viewport scissor(
				draw.mScissorRect.x,
				viewportSize.y - draw.mScissorRect.y,
				draw.mScissorRect.z,
				draw.mScissorRect.w
			);

			fg.pass(outTarget, sceneViewport, scissor, passState, shaderHandle)
				.with([draw, ortho_projection, viewportOffset, viewportSize](Pass& pass, const ResourceManagers& resourceManagers, const ECS& ecs) {
				pass.bindUniform("P", ortho_projection);
				pass.bindTexture("Texture", draw.mTextureHandle);
				pass.drawCommand(draw.mMeshHandle, {}, {}, draw.mElementCount, draw.mElementBufferOffset);
					})
				.dependsOn(std::forward<Deps>(deps)...)
				.setDebugName("Draw ImGui")
				;
		}

	}

	template<typename... Deps>
	inline void _drawImGuiMeshViews(
		FrameGraph& fg,
		FramebufferHandle outTarget,
		Viewport sceneViewport,
		const ResourceManagers& resourceManagers,
		const ECS& ecs,
		glm::uvec2 viewportOffset,
		glm::uvec2 viewportSize,
		Deps... deps
	) {
		TRACY_ZONE();

		auto shaderHandle = resourceManagers.mShaderManager.asyncLoad("ImGuiMeshShader", SourceShader::ConstructionArgs{
			{ types::shader::Stage::Vertex, "model.vert"},
			{ types::shader::Stage::Fragment, "forwardpbr.frag" }
			});

		const auto& meshView = ecs.getView<ImGuiMeshViewComponent, ImGuiComponent>();
		for (const ECS::Entity& meshViewEntity : meshView) {
			const ImGuiMeshViewComponent& drawComponent = meshView.get<ImGuiMeshViewComponent>(meshViewEntity);
			Viewport drawVP(0, 0, 175, 175);
			auto targetHandle = resourceManagers.mFramebufferManager.asyncLoad("MeshView",
				FramebufferBuilder{}
				.setSize(glm::uvec2(drawVP.z, drawVP.w))
				.attach(TextureFormat{ types::texture::Target::Texture2D, types::texture::InternalFormats::RGB8_UNORM })
				.attach(TextureFormat{ types::texture::Target::Texture2D, types::texture::InternalFormats::D16 }),
				resourceManagers.mTextureManager
			);

			fg.clear(targetHandle, glm::vec4(0.2f, 0.2f, 0.2f, 1.f), types::framebuffer::AttachmentBit::Color | types::framebuffer::AttachmentBit::Depth)
				.setDebugName("Clear ImGuiMesh View");
			fg.pass(targetHandle, drawVP, drawVP, {}, shaderHandle)
				.with([meshViewEntity, drawComponent](Pass& pass, const ResourceManagers& resourceManagers, const ECS& ecs) {
				TRACY_ZONEN("ImGuiMeshView");
				MakeDefine(DIRECTIONAL_LIGHT);
				pass.setDefine(DIRECTIONAL_LIGHT);

				// TODO - make static above
				pass.bindUniform("lightRadiance", glm::vec4(1.f));
				pass.bindUniform("lightDir", glm::vec3(0.f, 1.f, 0.f));

				BoundingBoxComponent bb(drawComponent.mMin, drawComponent.mMax);
				glm::vec3 camPos(0.f, 0.f, bb.getRadius() * 2.f);
				glm::vec3 camLook = bb.getCenter();
				pass.bindUniform("P", glm::perspective(45.f, 1.f, 0.1f, 10.f));
				pass.bindUniform("V", glm::lookAt(camPos, camLook, glm::vec3(0, 1, 0)));
				pass.bindUniform("camPos", camPos);

				pass.bindUniform("albedo", glm::vec4(1.f));
				pass.bindUniform("metalness", 0.f);
				pass.bindUniform("roughness", 0.f);
				pass.bindUniform("emissiveFactor", glm::vec3(0.f));

				// TODO - make static above
				pass.bindUniform("M", glm::mat4(1.f));
				pass.bindUniform("N", glm::mat3(1.f));

				pass.drawCommand(drawComponent.mMeshHandle, {}, {});
				})
				.setDebugName("ImGuiMeshView");

			Viewport vp;
			vp.x = drawComponent.mBounds.x;
			vp.z = drawComponent.mBounds.z - drawComponent.mBounds.x;
			vp.w = drawComponent.mBounds.w - drawComponent.mBounds.y;
			vp.y = viewportSize.y - drawComponent.mBounds.y - vp.w; // ImGui uses top left space
			blit(fg, vp, resourceManagers, targetHandle, outTarget, 0, std::forward<Deps>(deps)...);
		}
	}

#pragma warning(pop)

	}

	//#pragma warning(push)
	//#pragma warning (disable : 4100 )
	template<typename... Deps>
	inline void drawImGui(
		FrameGraph& fg,
		FramebufferHandle outTarget,
		Viewport sceneViewport,
		const ResourceManagers& resourceManagers,
		const ECS& ecs,
		glm::uvec2 viewportOffset,
		glm::uvec2 viewportSize,
		Deps... deps
	) {
		TRACY_ZONE();
		_drawImGuiMeshes<Deps...>(fg, outTarget, sceneViewport, resourceManagers, ecs, viewportOffset, viewportSize, std::forward<Deps>(deps)...);
		_drawImGuiMeshViews<Deps...>(fg, outTarget, sceneViewport, resourceManagers, ecs, viewportOffset, viewportSize, std::forward<Deps>(deps)...);

	}
//#pragma warning(pop)
}
