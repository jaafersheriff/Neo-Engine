#include "GLBackend/pch.hpp"

#include "GLBackend/RenderStateGL.hpp"
#include <GL/glew.h>
#include "Util/Assert.hpp"

namespace neo {
	namespace {
		GLenum _getGLBlendSrc(BlendFuncSrc src) {
			switch (src) {
			case BlendFuncSrc::One:
				return GL_ONE;
			case BlendFuncSrc::Alpha:
				return GL_SRC_ALPHA;
			default:
				NEO_FAIL("Invalid blend state");
				return GL_ONE;
			}
		}

		GLenum _getGLBlendDst(BlendFuncDst dst) {
			switch (dst) {
			case BlendFuncDst::One:
				return GL_ONE;
			case BlendFuncDst::OneMinusSrcAlpha:
				return GL_ONE_MINUS_SRC_ALPHA;
			default:
				NEO_FAIL("Invalid blend state");
				return GL_ONE;
			}
		}
	}

	void applyRenderState(const RenderState& renderState, const glm::uvec2& viewport) {
		glViewport(0, 0, viewport.x, viewport.y);

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

			if (renderState.mBlendState->mBlendAlpha) {
				glBlendFuncSeparate(
					_getGLBlendSrc(renderState.mBlendState->mBlendSrc),
					_getGLBlendDst(renderState.mBlendState->mBlendDst),
					_getGLBlendSrc(renderState.mBlendState->mBlendAlpha->mBlendSrc),
					_getGLBlendDst(renderState.mBlendState->mBlendAlpha->mBlendDst)
				);
			}
			else {
				glBlendFunc(_getGLBlendSrc(renderState.mBlendState->mBlendSrc), _getGLBlendDst(renderState.mBlendState->mBlendDst));
			}

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

		if (renderState.mScissor) {
			glEnable(GL_SCISSOR_TEST);
			glScissor(renderState.mScissor->x, renderState.mScissor->y, renderState.mScissor->z, renderState.mScissor->w);
		}
		else {
			glDisable(GL_SCISSOR_TEST);
		}

		switch (renderState.mPolygonMode) {
		case PolygonMode::Point:
			glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
			break;
		case PolygonMode::Line:
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			break;
		case PolygonMode::Fill:
		default:
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			break;
		}
	}
}