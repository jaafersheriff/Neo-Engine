#include "FrameData.hpp"

#include "Renderer/FrameGraph/FrameGraph.hpp"

namespace neo {

	FrameData::FrameData() {
		TRACY_ZONE();
		mUBOs = reinterpret_cast<UniformBuffer*>(calloc(4096, sizeof(UniformBuffer)));
		NEO_ASSERT(mUBOs, "Can't alloc");
		mShaderDefines = reinterpret_cast<ShaderDefinesFG*>(calloc(4096, sizeof(ShaderDefinesFG)));
		NEO_ASSERT(mShaderDefines, "Can't alloc");
		mPassStates = reinterpret_cast<PassState*>(calloc(4096, sizeof(PassState)));
		NEO_ASSERT(mPassStates, "Can't alloc");
		mFramebufferHandles = reinterpret_cast<FramebufferHandle*>(malloc(256 * sizeof(FramebufferHandle)));
		NEO_ASSERT(mFramebufferHandles, "Can't alloc");
		mViewports = reinterpret_cast<Viewport*>(malloc(256 * sizeof(Viewport)));
		NEO_ASSERT(mViewports, "Can't alloc");
	}

	FrameData::~FrameData() {
		TRACY_ZONE();
		{
			TRACY_ZONEN("Destroy UBO");
			for (int i = 0; i < mUBOIndex; i++) {
				mUBOs[i].destroy();
			}
		}
		{
			TRACY_ZONEN("Destroy Shader Defines");
			for (int i = 0; i < mShaderDefinesIndex; i++) {
				mShaderDefines[i].destroy();
			}
		}
		{
			TRACY_ZONEN("Dealloc");
			free(reinterpret_cast<void*>(mUBOs));
			free(reinterpret_cast<void*>(mShaderDefines));
			free(reinterpret_cast<void*>(mPassStates));
			free(reinterpret_cast<void*>(mFramebufferHandles));
			free(reinterpret_cast<void*>(mViewports));
		}
	}


	uint16_t FrameData::addPass(FramebufferHandle handle, Viewport vp, Viewport scissor, PassState& state, ShaderHandle shaderHandle) {
		mFramebufferHandles[mFramebufferHandleIndex] = handle;
		mShaderHandles[mShaderHandleIndex] = shaderHandle;
		auto vpId = mViewportIndex;
		mViewports[mViewportIndex++] = vp;
		auto scId = vpId;
		if (vp != scissor) {
			scId = mViewportIndex;
			mViewports[mViewportIndex++] = scissor;
		}
		{
			TRACY_ZONEN("Construct pass");
			mPasses.emplace_back(std::make_unique<Pass>(*this, mFramebufferHandleIndex++, vpId, scId, mShaderHandleIndex++, state));
		}
		return static_cast<uint16_t>(mPasses.size() - 1);
	}

	Pass& FrameData::getPass(uint16_t index) {
		NEO_ASSERT(mPasses.size() > index, "Invalid index");
		return *mPasses[index];
	}

	FramebufferHandle& FrameData::getFrameBufferHandle(uint8_t index) {
		NEO_ASSERT(mFramebufferHandleIndex > index, "Invalid index");
		return mFramebufferHandles[index];
	}

	Viewport& FrameData::getViewport(uint8_t index) {
		NEO_ASSERT(mViewportIndex > index, "Invalid index");
		return mViewports[index];
	}

	ShaderHandle& FrameData::getShaderHandle(uint8_t index) {
		NEO_ASSERT(mShaderHandleIndex > index, "Invalid index");
		return mShaderHandles[index];
	}

	UniformBuffer& FrameData::getUBO(uint16_t index) {
		NEO_ASSERT(mUBOIndex > index, "Invalid index");
		return mUBOs[index];
	}

	ShaderDefinesFG& FrameData::getDefines(uint16_t index) {
		NEO_ASSERT(mShaderDefinesIndex > index, "Invalid index");
		return mShaderDefines[index];
	}

	PassState& FrameData::getPassState(uint16_t index) {
		NEO_ASSERT(mPassStateIndex > index, "Invalid index");
		return mPassStates[index];
	}

	const FramebufferHandle& FrameData::getFrameBufferHandle(uint8_t index) const {
		NEO_ASSERT(mFramebufferHandleIndex > index, "Invalid index");
		return mFramebufferHandles[index];
	}

	const Viewport& FrameData::getViewport(uint8_t index) const {
		NEO_ASSERT(mViewportIndex > index, "Invalid index");
		return mViewports[index];
	}

	const ShaderHandle& FrameData::getShaderHandle(uint8_t index) const {
		NEO_ASSERT(mShaderHandleIndex > index, "Invalid index");
		return mShaderHandles[index];
	}

	const UniformBuffer& FrameData::getUBO(uint16_t index) const {
		NEO_ASSERT(mUBOIndex > index, "Invalid index");
		return mUBOs[index];
	}

	const ShaderDefinesFG& FrameData::getDefines(uint16_t index) const {
		NEO_ASSERT(mShaderDefinesIndex > index, "Invalid index");
		return mShaderDefines[index];
	}

	const PassState& FrameData::getPassState(uint16_t index) const {
		NEO_ASSERT(mPassStateIndex > index, "Invalid index");
		return mPassStates[index];
	}

	uint16_t FrameData::createUBO(const UniformBuffer& ubo) {
		mUBOs[mUBOIndex] = ubo;
		return mUBOIndex++;
	}

	uint16_t FrameData::createShaderDefines(const ShaderDefinesFG& defines) {
		mShaderDefines[mShaderDefinesIndex] = defines;
		return mShaderDefinesIndex++;
	}

	uint16_t FrameData::createPassState(const PassState& passState) {
		mPassStates[mPassStateIndex] = passState;
		return mPassStateIndex++;
	}
}