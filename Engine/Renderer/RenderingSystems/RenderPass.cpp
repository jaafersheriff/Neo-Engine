#include "Renderer/pch.hpp"

#include "Renderer/FrameStats.hpp"
#include "RenderPass.hpp"

namespace neo {
	namespace {
		// TODO - move this to its own func maybe
		void _applyState(const RenderState& renderState) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, 0);
			glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
			glBindVertexArray(0);
			glUseProgram(0);

			if (renderState.mDepthState) {
				glEnable(GL_DEPTH_TEST);
				switch (renderState.mDepthState->mDepthFunc) {
					case DepthFunc::Less:
						glDepthFunc(GL_LESS);
						break;
					case DepthFunc::LessEqual:
						glDepthFunc(GL_LEQUAL);
						break;
					default:
						NEO_FAIL("Invalid depth func");
						break;
				}
				glDepthMask(renderState.mDepthState->mDepthMask);
			}
			else {
				glDisable(GL_DEPTH_TEST);
			}

			if (renderState.mCullFace) {
				glEnable(GL_CULL_FACE);
				switch (renderState.mCullFace.value()) {
				case CullFace::Back:
					glCullFace(GL_BACK);
					break;
				case CullFace::Front:
					glCullFace(GL_FRONT);
					break;
				default:
					NEO_FAIL("Invalid cull face");
					break;
				}
			}
			else {
				glDisable(GL_CULL_FACE);
			}

			if (renderState.mBlendState) {
				glEnable(GL_BLEND);
				switch (renderState.mBlendState->mBlendEquation) {
				case BlendEquation::Add:
					glBlendEquation(GL_FUNC_ADD);
					break;
				default:
					NEO_FAIL("Invalid blend equation");
					break;
				}

				uint32_t blendSrc = 0;
				uint32_t blendDst = 0;
				switch (renderState.mBlendState->mBlendSrc) {
				case BlendFuncSrc::One:
					blendSrc = GL_ONE;
					break;
				case BlendFuncSrc::Alpha:
					blendSrc = GL_SRC_ALPHA;
					break;
				default:
					NEO_FAIL("Invalid blend state");
					break;
				}
				switch (renderState.mBlendState->mBlendDst) {
				case BlendFuncDst::One:
					blendDst = GL_ONE;
					break;
				case BlendFuncDst::OneMinusSrcAlpha:
					blendDst = GL_ONE_MINUS_SRC_ALPHA;
					break;
				default:
					NEO_FAIL("Invalid blend state");
					break;
				}
				glBlendFunc(blendSrc, blendDst);

				glBlendColor(
					renderState.mBlendState->mBlendColor.r,
					renderState.mBlendState->mBlendColor.g,
					renderState.mBlendState->mBlendColor.b,
					renderState.mBlendState->mBlendColor.a
				);
			}
			else {
				glDisable(GL_BLEND);
			}

			if (renderState.mWireframe) {
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			}
			else {
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			}
		}
	}
	void RenderPasses::clear(FramebufferHandle target, types::framebuffer::AttachmentBits clearFlags, glm::vec4 clearColor, std::optional<std::string> debugName) {
		mPasses.emplace_back(ClearPass{
			target,
			clearFlags,
			clearColor,
			debugName
		});
	}

	void RenderPasses::renderPass(FramebufferHandle target, glm::uvec2 viewport, RenderState renderState, DrawFunction draw, std::optional<std::string> debugName) {
		renderState.mWireframe = mWireframeOverride;
		mPasses.emplace_back(RenderPass{
			target,
			viewport,
			renderState,
			draw,
			debugName
		});
	}

	void RenderPasses::computePass(DrawFunction draw, std::optional<std::string> debugName) {
		mPasses.emplace_back(ComputePass{
			draw,
			debugName
		});
	}

	void RenderPasses::_execute(FrameStats& renderStats, const ResourceManagers& resourceManagers, const ECS& ecs) {
		renderStats.mRenderPasses.clear();
		TRACY_GPU();
		for (const auto& pass : mPasses) {
			util::visit(pass,
				[&](const ComputePass& computePass) {
					renderStats.mRenderPasses.emplace_back(computePass.mDebugName.value_or("Nameless compute pass"));

					computePass.mDrawFunction(resourceManagers, ecs);
				},
				[&](const RenderPass& renderPass) {
					renderStats.mRenderPasses.emplace_back(renderPass.mDebugName.value_or("Nameless compute pass"));

					if (!resourceManagers.mFramebufferManager.isValid(renderPass.mTarget)) {
						NEO_LOG_W("Unable to resolve target, skipping pass %s", renderPass.mDebugName.value_or("").c_str());
						return;
					}
					resourceManagers.mFramebufferManager.resolve(renderPass.mTarget).bind();
					glViewport(0, 0, renderPass.mViewport.x, renderPass.mViewport.y);

					_applyState(renderPass.mRenderState);
					renderPass.mDrawFunction(resourceManagers, ecs);
				},
				[&](const ClearPass& clearPass) {
					renderStats.mRenderPasses.emplace_back(clearPass.mDebugName.value_or("Nameless render pass"));

					TRACY_GPUN("Clear");
					if (!resourceManagers.mFramebufferManager.isValid(clearPass.mTarget)) {
						NEO_LOG_W("Unable to resolve target, skipping clear %s", clearPass.mDebugName.value_or("").c_str());
						return;
					}
					resourceManagers.mFramebufferManager.resolve(clearPass.mTarget).bind();
					resourceManagers.mFramebufferManager.resolve(clearPass.mTarget).clear(clearPass.mClearColor, clearPass.mClearFlags);

				},
				[&](auto) { static_assert(always_false_v<T>, "non-exhaustive visitor!"); }
			);
		}
	}
}