#include "GLFrameGraphResolve.hpp"

namespace neo {

	namespace {

		void _startPass(const FrameData& frameData, const Pass& pass, const Command& command, const ResourceManagers& resourceManagers) {
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

			const PassState& passState = frameData.getPassState(pass.mPassStateIndex);
			if (passState.mDepthTest) {
				glEnable(GL_DEPTH_TEST);
				switch (passState.mDepthFunc) {
				case types::passState::DepthFunc::Less:
					glDepthFunc(GL_LESS);
					break;
				case types::passState::DepthFunc::LessEqual:
					glDepthFunc(GL_LEQUAL);
					break;
				default:
					NEO_FAIL("Invalid");
					break;
				}
			}
			else {
				glDisable(GL_DEPTH_TEST);
			}
			glDepthMask(passState.mDepthMask ? GL_TRUE : GL_FALSE);
			if (passState.mCullFace) {
				glEnable(GL_CULL_FACE);
				switch (passState.mCullOrder) {
				case types::passState::CullOrder::Front:
					glCullFace(GL_FRONT);
					break;
				case types::passState::CullOrder::Back:
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
			if (passState.mBlending) {
				glEnable(GL_BLEND);
				switch (passState.mBlendEquation) {
				case types::passState::BlendEquation::Add:
					glBlendEquation(GL_FUNC_ADD);
					break;
				default:
					NEO_FAIL("Invalid");
					break;
				}

				auto getBlendFactor = [](types::passState::BlendFactor factor) {
					switch (factor) {
					case types::passState::BlendFactor::Alpha:
						return GL_SRC_ALPHA;
					case types::passState::BlendFactor::OneMinusAlpha:
						return GL_ONE_MINUS_SRC_ALPHA;
					case types::passState::BlendFactor::One:
						return GL_ONE;
					default:
						NEO_FAIL("Invalid");
						return 0;
					}
					};
				glBlendFuncSeparate(
					getBlendFactor(passState.mBlendSrcRGB),
					getBlendFactor(passState.mBlendDstRGB),
					getBlendFactor(passState.mBlendSrcAlpha),
					getBlendFactor(passState.mBlendDstAlpha)
				);
			}
			else {
				glDisable(GL_BLEND);
			}
			if (passState.mStencilTest) {
				NEO_LOG_E("Stencil Test unsupported");
			}
			else {
				glDisable(GL_STENCIL_TEST);
			}
			if (passState.mScissorTest) {
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
			const auto& ubo = frameData.getUBO(static_cast<uint16_t>(
				command >> (64 - 3 - 10 - 10) & 0b1111111111
			));
			const auto& drawDefines = frameData.getDefines(static_cast<uint16_t>(
				command >> (64 - 3 - 10 - 10 - 10) & 0b1111111111
			));

			const auto& resolvedShader = resourceManagers.mShaderManager.resolveDefines(shaderHandle, {frameData.getDefines(pass.mPassDefinesIndex), drawDefines});
			const UniformBuffer& passUniforms = frameData.getUBO(pass.mPassUBOIndex);
			for (uint8_t i = 0; i < passUniforms.getUniformsSize(); i++) {
				const auto pair = passUniforms.getUniform(i);
				resolvedShader.bindUniform(pair.first, pair.second);
			}
			for (uint8_t i = 0; i < ubo.getUniformsSize(); i++) {
				const auto pair = ubo.getUniform(i);
				resolvedShader.bindUniform(pair.first, pair.second);
			}
			for (uint8_t i = 0; i < passUniforms.getTextureBindSize(); i++) {
				const auto pair = passUniforms.getTexture(i);
				if (resourceManagers.mTextureManager.isValid(pair.second)) {
					resolvedShader.bindTexture(pair.first, resourceManagers.mTextureManager.resolve(pair.second));
				}
			}
			for (uint8_t i = 0; i < ubo.getTextureBindSize(); i++) {
				const auto pair = ubo.getTexture(i);
				if (resourceManagers.mTextureManager.isValid(pair.second)) {
					resolvedShader.bindTexture(pair.first, resourceManagers.mTextureManager.resolve(pair.second));
				}
			}

			resourceManagers.mMeshManager.resolve(draw.mMeshHandle).draw(draw.mElementCount, draw.mElementBufferOffset);
		}
	}

	void GLFrameGraphResolve(const FrameData& frameData, const Pass& pass, const Command& command, const ResourceManagers& resourceManagers) {
		// This should just be one big for loop tbh
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