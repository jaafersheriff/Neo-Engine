#pragma once

#include "Renderer/GLObjects/PassState.hpp"
#include "Renderer/GLObjects/UniformBuffer.hpp"
#include "ResourceManager/ResourceManagers.hpp"
#include "Renderer/FrameGraph/ShaderDefinesFG.hpp"

namespace neo {
	struct Pass;
	struct FrameData {
		FrameData();
		~FrameData();

		uint16_t addPass(FramebufferHandle handle, Viewport vp, Viewport scissor, PassState& state, ShaderHandle shaderHandle = NEO_INVALID_HANDLE);
		Pass& getPass(uint16_t index);

		FramebufferHandle& getFrameBufferHandle(uint8_t index);
		Viewport& getViewport(uint8_t index);
		ShaderHandle& getShaderHandle(uint8_t index);
		UniformBuffer& getUBO(uint16_t index);
		ShaderDefinesFG& getDefines(uint16_t index);
		PassState& getPassState(uint16_t index);

		// Do these need to be explicit..?
		const FramebufferHandle& getFrameBufferHandle(uint8_t index) const;
		const Viewport& getViewport(uint8_t index) const;
		const ShaderHandle& getShaderHandle(uint8_t index) const ;
		const UniformBuffer& getUBO(uint16_t index) const ;
		const ShaderDefinesFG& getDefines(uint16_t index) const ;
		const PassState& getPassState(uint16_t index) const ;

		uint16_t createUBO(const UniformBuffer& ubo);
		uint16_t createShaderDefines(const ShaderDefinesFG& defines);
		uint16_t createPassState(const PassState& passState);

	private:
		FramebufferHandle* mFramebufferHandles;
		uint8_t mFramebufferHandleIndex = 0;

		Viewport* mViewports;
		uint8_t mViewportIndex = 0;

		ShaderHandle mShaderHandles[256];
		uint8_t mShaderHandleIndex = 0;

		UniformBuffer* mUBOs = nullptr;
		uint16_t mUBOIndex = 0; // 12 bits, max of 4096

		ShaderDefinesFG* mShaderDefines = nullptr;
		uint16_t mShaderDefinesIndex = 0; // 12 bits, max of 4096

		PassState* mPassStates = nullptr;
		uint16_t mPassStateIndex = 0; // 12 bits, max of 4096

	private:
		std::vector<std::unique_ptr<Pass>> mPasses;
	};

}