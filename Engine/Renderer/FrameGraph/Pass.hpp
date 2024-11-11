#pragma once

namespace neo {

	using Command = uint64_t;
	enum class CommandType : uint8_t {
		StartPass = 0,
		Clear = 1,
		Draw = 2
	};

	struct Pass {
		Pass(FrameData& frameData, uint8_t fbID, uint8_t vpID, uint8_t scId, uint8_t shaderID, PassState& state)
			: mFrameData(frameData)
			, mFramebufferIndex(fbID)
			, mViewportIndex(vpID)
			, mScissorIndex(scId)
			, mShaderIndex(shaderID)
		{
			TRACY_ZONE();
			{
				TRACY_ZONEN("FrameData copies");
				mPassStateIndex = mFrameData.createPassState(state);
				mPassUBOIndex = mFrameData.createUBO({});
				mPassDefinesIndex = mFrameData.createShaderDefines({});
			}
			{
				TRACY_ZONEN("Commands Alloc");
				mCommands = reinterpret_cast<Command*>(malloc(1024 * sizeof(Command)));
				NEO_ASSERT(mCommands, "Can't alloc");
			}
			{
				TRACY_ZONEN("Draws Alloc");
				mDraws = reinterpret_cast<Draw*>(malloc(1024 * sizeof(Draw)));
				NEO_ASSERT(mDraws, "Can't alloc");
			}

		}

		~Pass() {
			TRACY_ZONE();
			free(reinterpret_cast<void*>(mCommands));
			free(reinterpret_cast<void*>(mDraws));
		}

		// 0-3: StartPass Key
		// 4-12: FBO ID
		// 13-21: Viewport ID
		void startCommand() {
			Command& command = mCommands[mCommandIndex++];
			command = 0
				| static_cast<uint64_t>(CommandType::StartPass) << (64 - 3)
				| static_cast<uint64_t>(mFramebufferIndex) << (64 - 3 - 8)
				| static_cast<uint64_t>(mViewportIndex) << (64 - 3 - 8 - 8)
			;
		}

		void drawCommand(const MeshHandle& mesh, const UniformBuffer& ubo, const ShaderDefinesFG& drawDefines, uint16_t elements = 0, uint16_t bufferOffset = 0) {
			uint64_t drawIndex = mDrawIndex;
			mDraws[mDrawIndex++] = { mesh, elements, bufferOffset };

			// Bad copies :(
			uint64_t uboIndex = mFrameData.createUBO(ubo);
			uint64_t definesIndex = mFrameData.createShaderDefines(drawDefines);

			Command& command = mCommands[mCommandIndex++];
			command = 0
				| static_cast<uint64_t>(CommandType::Draw) << (64 - 3)
				| drawIndex << (64 - 3 - 10)
				| uboIndex << (64 - 3 - 10 - 10)
				| definesIndex << (64 - 3 - 10 - 10 - 10)
			;
		}


		void setDefine(const ShaderDefine& define) {
			mFrameData.getDefines(mPassDefinesIndex).set(define);
		}

		void bindUniform(const char* name, const ResolvedShaderInstance::UniformVariant& variant) {
			mFrameData.getUBO(mPassUBOIndex).bindUniform(name, variant);
		}
		void bindTexture(const char* name, TextureHandle handle) {
			mFrameData.getUBO(mPassUBOIndex).bindTexture(name, handle);
		}

		struct ClearParams {
			glm::vec4 color;
			types::framebuffer::AttachmentBits clearFlags = 0;
		};
		void clearCommand(glm::vec4 color, types::framebuffer::AttachmentBits clearFlags) {
			mClearParams[mClearParamsIndex] = ClearParams{ color, clearFlags };

			Command& command = mCommands[mCommandIndex++];
			command = 0
				| static_cast<uint64_t>(CommandType::Clear) << (64 - 3)
				| static_cast<uint64_t>(mClearParamsIndex) << (64 - 3 - 3)
			;
			mClearParamsIndex++;
		}

		uint16_t getCommandSize() const { return mCommandIndex; }
		Command& getCommand(uint16_t index) { return mCommands[index]; }
		
	//private:
		FrameData& mFrameData;

		uint8_t mFramebufferIndex = 0;
		uint8_t mViewportIndex = 0;
		uint8_t mScissorIndex = 0;
		uint8_t mShaderIndex = 0;
		uint16_t mPassStateIndex = 0;
		uint16_t mPassUBOIndex = 0;
		uint16_t mPassDefinesIndex = 0;

		Command* mCommands; // 1024 max 
		uint16_t mCommandIndex = 0;

		ClearParams mClearParams[8];
		uint8_t mClearParamsIndex = 0; // 3 bits

		struct Draw {
			MeshHandle mMeshHandle;
			uint16_t mElementCount = 0;
			uint16_t mElementBufferOffset = 0;
		};
		Draw* mDraws; // 1024 max
		uint16_t mDrawIndex = 0; // 10 bits
	};

}