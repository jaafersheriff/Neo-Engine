#include "FrameData.hpp"

#include "Renderer/FrameGraph/FrameGraph.hpp"

namespace neo {

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

	FrameData::FrameData() {
		TRACY_ZONE();
		{
			TRACY_ZONEN("UBO Alloc");
			mUBOs = reinterpret_cast<UniformBuffer*>(malloc(4096 * sizeof(UniformBuffer)));
			NEO_ASSERT(mUBOs, "Can't alloc");
		}
		{
			TRACY_ZONEN("UBO Reset");
			for (int i = 0; i < 4096; i++) {
				mUBOs[i].reset();
			}
		}
		{
			TRACY_ZONEN("Defines Alloc");
			mShaderDefines = reinterpret_cast<ShaderDefinesFG*>(malloc(4096 * sizeof(ShaderDefinesFG)));
			NEO_ASSERT(mShaderDefines, "Can't alloc");
		}
		{
			TRACY_ZONEN("Defines Reset");
			for (int i = 0; i < 4096; i++) {
				mShaderDefines[i].reset();
			}
		}
	}

	uint16_t FrameData::createUBO(const UniformBuffer& ubo) {
		mUBOs[mUBOIndex] = ubo;
		return mUBOIndex++;
	}

	uint16_t FrameData::createShaderDefines(const ShaderDefinesFG& defines) {
		mShaderDefines[mShaderDefinesIndex] = defines;
		return mShaderDefinesIndex++;
	}

	FrameData::~FrameData() {
		free(mUBOs);
		free(mShaderDefines);
	}
}