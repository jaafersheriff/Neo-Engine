#include "GLFrameGraphResolve.hpp"

namespace neo {

	namespace {

		void _startPass(const FrameData& frameData, const  Pass& pass, const Command& command, const ResourceManagers& resourceManagers) {
			uint8_t fbID = static_cast<uint8_t>(
				command >> (64 - 3 - 8) & 0xFF
				);
			uint8_t vpID = static_cast<uint8_t>(
				command >> (64 - 3 - 8 - 8) & 0xFF
				);

			const auto& fbHandle = frameData.getFrameBufferHandle(fbID);
			const auto& vp = frameData.getViewport(vpID);

			if (resourceManagers.mFramebufferManager.isValid(fbHandle)) {
				resourceManagers.mFramebufferManager.resolve(fbHandle).bind();
				glViewport(vp.x, vp.y, vp.z, vp.w);
			}

			if (pass.mPassState.mDepthTest) {
				glEnable(GL_DEPTH_TEST);
				switch (pass.mPassState.mDepthFunc) {
				case DepthFunc::Less:
					glDepthFunc(GL_LESS);
					break;
				default:
					NEO_FAIL("Invalid");
					break;
				}
			}
			else {
				glDisable(GL_DEPTH_TEST);
			}
			glDepthMask(pass.mPassState.mDepthMask ? GL_TRUE : GL_FALSE);
			if (pass.mPassState.mCullFace) {
				glEnable(GL_CULL_FACE);
				switch (pass.mPassState.mCullOrder) {
				case CullOrder::Front:
					glCullFace(GL_FRONT);
					break;
				case CullOrder::Back:
					glCullFace(GL_BACK);
					break;
				default:
					NEO_FAIL("Invalid");
					break;
				}
			}
			else {
				glDisable(GL_CULL_FACE);
			}
			if (pass.mPassState.mBlending) {
				glEnable(GL_BLEND);
				switch (pass.mPassState.mBlendEquation) {
				case BlendEquation::Add:
					glBlendEquation(GL_FUNC_ADD);
					break;
				default:
					NEO_FAIL("Invalid");
					break;
				}

				auto getBlendFactor = [](BlendFactor factor) {
					switch (factor) {
					case BlendFactor::Alpha:
						return GL_SRC_ALPHA;
					case BlendFactor::OneMinusAlpha:
						return GL_ONE_MINUS_SRC_ALPHA;
					case BlendFactor::One:
						return GL_ONE;
					default:
						NEO_FAIL("Invalid");
						return 0;
					}
					};
				glBlendFuncSeparate(
					getBlendFactor(pass.mPassState.mBlendSrcRGB),
					getBlendFactor(pass.mPassState.mBlendDstRGB),
					getBlendFactor(pass.mPassState.mBlendSrcAlpha),
					getBlendFactor(pass.mPassState.mBlendDstAlpha)
				);
			}
			else {
				glDisable(GL_BLEND);
			}
			if (pass.mPassState.mStencilTest) {
				NEO_LOG_E("Stencil Test unsupported");
			}
			else {
				glDisable(GL_STENCIL_TEST);
			}
			if (pass.mPassState.mScissorTest) {
				glEnable(GL_SCISSOR_TEST);
				Viewport scissor = frameData.getViewport(pass.mScissorIndex);
				glScissor(scissor.x, scissor.y, scissor.z, scissor.w);
			}
			else {
				glDisable(GL_SCISSOR_TEST);
			}
		}

		void _clear(const Pass& pass, const Command& command) {
			const auto& clearParams = pass.mClearParams[static_cast<uint8_t>(
				command >> (64 - 3 - 3) & 0b111
				)];

			// temp framebuffer to make gl calls
			Framebuffer{}.clear(clearParams.color, clearParams.clearFlags);
		}

		void _draw(const FrameData& frameData, const Pass& pass, const Command& command, const ResourceManagers& resourceManagers) {
			const auto& shaderHandle = frameData.getShaderHandle(pass.mShaderIndex);
			if (!resourceManagers.mShaderManager.isValid(shaderHandle)) {
				return;
			}

			const auto& draw = pass.mDraws[static_cast<uint16_t>(
				command >> (64 - 3 - 10) & 0b1111111111
				)];
			if (!resourceManagers.mMeshManager.isValid(draw.mMeshHandle)) {
				return;
			}
			const auto& ubo = pass.mUBOs[static_cast<uint16_t>(
				command >> (64 - 3 - 10 - 10) & 0b1111111111
				)];
			const auto& drawDefines = pass.mShaderDefines[static_cast<uint16_t>(
				command >> (64 - 3 - 10 - 10 - 10) & 0b1111111111
				)];

			ShaderDefines defines;
			pass.mPassDefines.toOldStyle(defines);
			drawDefines.toOldStyle(defines);

			const auto& resolvedShader = resourceManagers.mShaderManager.resolveDefines(shaderHandle, defines);
			for (auto& [k, v] : pass.mPassUBO.mUniforms) {
				resolvedShader.bindUniform(k, v);
			}
			for (auto& [k, t] : pass.mPassUBO.mTextures) {
				if (resourceManagers.mTextureManager.isValid(t)) {
					resolvedShader.bindTexture(k, resourceManagers.mTextureManager.resolve(t));
				}
			}
			for (auto& [k, v] : ubo.mUniforms) {
				resolvedShader.bindUniform(k, v);
			}
			for (auto& [k, t] : ubo.mTextures) {
				if (resourceManagers.mTextureManager.isValid(t)) {
					resolvedShader.bindTexture(k, resourceManagers.mTextureManager.resolve(t));
				}
			}

			resourceManagers.mMeshManager.resolve(draw.mMeshHandle).draw(draw.mElementCount, draw.mElementBufferOffset);
		}
	}

	void GLFrameGraphResolve(const FrameData& frameData, const Pass& pass, const Command& command, const ResourceManagers& resourceManagers) {
		CommandType type = static_cast<CommandType>(command >> (64 - 3) & 0b111);
		switch (type) {
		case CommandType::Clear:
			_clear(pass, command);
			break;
		case CommandType::Draw:
			_draw(frameData, pass, command, resourceManagers);
			break;
		case CommandType::StartPass:
			_startPass(frameData, pass, command, resourceManagers);
			break;
		default:
			NEO_FAIL("Invalid pass type");
			break;
		}
	}
}