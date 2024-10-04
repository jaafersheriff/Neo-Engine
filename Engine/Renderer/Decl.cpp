#include "Decl.hpp"

namespace neo {
	namespace {
		void _executeClear(const ResourceManagers& resourceManagers, const ClearCommand& clearCommand) {
			TRACY_GPU();
			if (resourceManagers.mFramebufferManager.isValid(clearCommand.handle)) {
				auto& target = resourceManagers.mFramebufferManager.resolve(clearCommand.handle);

				target.bind();
				target.clear(clearCommand.clearColor, clearCommand.clearFlags);
			}
		}

		void _executeRenderPass(const ResourceManagers& resourceManagers, const RenderPass& renderPass) {
			TRACY_GPU();
			if (renderPass.state.blendState == BlendState::Disabled) {
				glDisable(GL_BLEND);
			}
			else {
				glEnable(GL_BLEND);
				glBlendEquation(GL_FUNC_ADD);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			}

			glViewport(renderPass.state.viewport.x, renderPass.state.viewport.y, renderPass.state.viewport.z, renderPass.state.viewport.w);

			resourceManagers.mFramebufferManager.resolve(renderPass.fboHandle).bind();

			// Yikes
			ShaderDefines passDefines;
			for (auto& defineString : renderPass.passDefines.defines) {
				passDefines.set(ShaderDefine(defineString.c_str()));
			}

			ShaderDefines drawDefines(passDefines);
			for (int i = 0; i < renderPass.drawCount; i++) {
				const auto& draw = renderPass.draws[i];
				drawDefines.reset();

				// Yikes
				for (auto& define : draw.drawDefines.defines) {
					drawDefines.set(define.c_str());
				}

				auto& resolvedShader = resourceManagers.mShaderManager.resolveDefines(renderPass.shaderHandle, drawDefines);
				resolvedShader.bind();
				for (auto&& [hash, data] : renderPass.passUniforms.mUniforms) {
					resolvedShader.bindUniform(hash, data);
				}
				for (auto&& [hash, data] : draw.uniforms.mUniforms) {
					resolvedShader.bindUniform(hash, data);
				}

				for (int j = 0; j < renderPass.passTextures.textureCount; j++) {
					resolvedShader.bindTexture(renderPass.passTextures.mTextures[j].first, resourceManagers.mTextureManager.resolve(renderPass.passTextures.mTextures[j].second));
				}
				for (int j = 0; j < draw.textures.textureCount; j++) {
					resolvedShader.bindTexture(draw.textures.mTextures[j].first, resourceManagers.mTextureManager.resolve(draw.textures.mTextures[j].second));
				}

				resourceManagers.mMeshManager.resolve(draw.mesh).draw();
			}

		}
	}

	ClearCommand& Decl::declareClearCommand(FramebufferHandle handle, glm::vec4 clearColor, types::framebuffer::AttachmentBits clearFlags) {
		ClearCommand clear;
		clear.handle = handle;
		clear.clearColor = clearColor;
		clear.clearFlags = clearFlags;
		return std::get<ClearCommand>(mCommands.emplace_back(std::move(clear)));
	}

	RenderPass& Decl::declareRenderPass(FramebufferHandle target, glm::uvec4 viewport) {
		auto& pass = std::get<RenderPass>(mCommands.emplace_back(std::move(RenderPass{})));
		pass.fboHandle = target;
		pass.state.viewport = viewport;
		return pass;
	}

	void Decl::execute(const ResourceManagers& resourceManagers) {
		TRACY_GPU();
		for (auto& command : mCommands) {
			util::visit(command,
				[&](ClearCommand& clearCommand) {
					_executeClear(resourceManagers, clearCommand);
				},
				[&](RenderPass& renderPass) {
					_executeRenderPass(resourceManagers, renderPass);
				},
				[&](auto) { static_assert(always_false_v<T>, "non-exhaustive visitor!"); }
			);
		}
	}
}