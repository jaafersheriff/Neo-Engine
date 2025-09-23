#include "Renderer/pch.hpp"

#include "Renderer/GLObjects/RenderStateGL.hpp"
#include <GL/glew.h>

namespace neo {
	void applyRenderState(const RenderState& renderState, bool wireframeOverride) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
		glBindVertexArray(0);
		glUseProgram(0);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

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

		if (wireframeOverride) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
	}
}