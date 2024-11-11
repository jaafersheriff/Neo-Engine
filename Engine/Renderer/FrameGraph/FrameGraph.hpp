#pragma once

#include "ECS/ECS.hpp"
#include "ResourceManager/ResourceManagers.hpp"
#include "Renderer/GLObjects/PassState.hpp"
#include "Renderer/GLObjects/UniformBuffer.hpp"
#include "Renderer/FrameGraph/ShaderDefinesFG.hpp"

#include <ext/entt_incl.hpp>

namespace neo {

	using Command = uint64_t;
	enum class CommandType : uint8_t {
		StartPass = 0,
		Clear = 1,
		Draw = 2
	};

	struct Pass {
		Pass(uint8_t fbID, uint8_t vpID, uint8_t scId, uint8_t shaderID, PassState& state)
			: mFramebufferIndex(fbID)
			, mViewportIndex(vpID)
			, mScissorIndex(scId)
			, mShaderIndex(shaderID)
			, mPassState(state)
		{
			TRACY_ZONE();
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
			uint64_t uboIndex = mUBOIndex;
			mUBOs[mUBOIndex++] = ubo;
			uint64_t definesIndex = mShaderDefinesIndex;
			mShaderDefines[mShaderDefinesIndex++] = drawDefines;

			Command& command = mCommands[mCommandIndex++];
			command = 0
				| static_cast<uint64_t>(CommandType::Draw) << (64 - 3)
				| drawIndex << (64 - 3 - 10)
				| uboIndex << (64 - 3 - 10 - 10)
				| definesIndex << (64 - 3 - 10 - 10 - 10)
			;
		}

		ShaderDefinesFG& createShaderDefines() {
			return mShaderDefines[mShaderDefinesIndex++];
		}
		void setDefine(const ShaderDefine& define) {
			mPassDefines.set(define);
		}

		UniformBuffer& createUBO() {
			return mUBOs[mUBOIndex++];
		}
		void bindUniform(const char* name, const ResolvedShaderInstance::UniformVariant& variant) {
			mPassUBO.bindUniform(name, variant);
		}
		void bindTexture(const char* name, TextureHandle handle) {
			mPassUBO.bindTexture(name, handle);
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

		const PassState& getPassState() const { return mPassState; }
		
		void destroy() {
			TRACY_ZONE();
			mPassUBO.destroy();
			for (int i = 0; i < mUBOIndex; i++) {
				mUBOs[i].destroy();
			}
		}
	//private:
		uint8_t mFramebufferIndex = 0;
		uint8_t mViewportIndex = 0;
		uint8_t mScissorIndex = 0;
		uint8_t mShaderIndex = 0;
		UniformBuffer mPassUBO;
		ShaderDefinesFG mPassDefines;
		PassState mPassState;

		Command mCommands[1024];
		uint16_t mCommandIndex = 0;

		ClearParams mClearParams[8];
		uint8_t mClearParamsIndex = 0; // 3 bits

		ShaderDefinesFG mShaderDefines[1024];
		uint16_t mShaderDefinesIndex = 0; // 10 bits

		UniformBuffer mUBOs[1024];
		uint16_t mUBOIndex = 0; // 10 bits

		struct Draw {
			MeshHandle mMeshHandle;
			uint16_t mElementCount = 0;
			uint16_t mElementBufferOffset = 0;
		};
		Draw mDraws[1024];
		uint16_t mDrawIndex = 0; // 10 bits
	};


	struct FrameData {

		uint16_t addPass(FramebufferHandle handle, Viewport vp, Viewport scissor, PassState& state, ShaderHandle shaderHandle = {}) {
			mFramebufferHandles[mFramebufferHandleIndex] = handle;
			mShaderHandles[mShaderHandleIndex] = shaderHandle;
			auto vpId = mViewportIndex;
			mViewports[mViewportIndex++] = vp;
			auto scId = vpId;
			if (vp != scissor) {
				scId = mViewportIndex;
				mViewports[mViewportIndex++] = scissor;
			}
			mPasses.emplace_back(mFramebufferHandleIndex++, vpId, scId, mShaderHandleIndex++, state);
			return static_cast<uint16_t>(mPasses.size() - 1);
		}

		Pass& getPass(uint16_t index) {
			NEO_ASSERT(mPasses.size() > index, "Invalid index");
			return mPasses[index];
		}

		const FramebufferHandle& getFrameBufferHandle(uint8_t index) const {
			NEO_ASSERT(mFramebufferHandleIndex > index, "Invalid index");
			return mFramebufferHandles[index];
		}

		const Viewport& getViewport(uint8_t index) const {
			NEO_ASSERT(mViewportIndex > index, "Invalid index");
			return mViewports[index];
		}

		const ShaderHandle& getShaderHandle(uint8_t index) const {
			NEO_ASSERT(mShaderHandleIndex > index, "Invalid index");
			return mShaderHandles[index];
		}

	private:
		FramebufferHandle mFramebufferHandles[256];
		uint8_t mFramebufferHandleIndex = 0;

		Viewport mViewports[256];
		uint8_t mViewportIndex = 0;

		ShaderHandle mShaderHandles[256];
		uint8_t mShaderHandleIndex = 0;

	private:
		std::vector<Pass> mPasses;
	};

	class FrameGraph {
	public:
		struct Task {
			using Functor = std::function<void(Pass&, const ResourceManagers&, const ECS&)>;
			Task()
				: mPassIndex(UINT16_MAX)
				, f([](Pass&, const ResourceManagers&, const ECS&) {})
			{}
			Task(uint16_t passIndex, Functor _f) :
				mPassIndex(passIndex),
				f(_f)
			{}

			uint16_t mPassIndex;
			Functor f;
			std::optional<std::string> mDebugName;
		};

		void execute(const ResourceManagers& resourceManagers, const ECS& ecs);

		template<typename... Deps>
		Task& pass(FramebufferHandle target, Viewport viewport, Viewport scissor, PassState state, ShaderHandle shader, Task::Functor t, Deps... deps) {
			TRACY_ZONE();
			uint16_t passIndex = mFrameData.addPass(target, viewport, scissor, state, shader);
			return _task(std::move(Task(passIndex, [_t = std::move(t)](Pass& pass, const ResourceManagers& resourceManager, const ECS& ecs) {
				pass.startCommand();
				_t(pass, resourceManager, ecs);
			})), target, std::forward<Deps>(deps)...);
		}

		Task& clear(FramebufferHandle handle, glm::vec4 color, types::framebuffer::AttachmentBits clearFlags) {
			return pass(handle, {}, {}, {}, {}, [=](Pass& pass, const ResourceManagers&, const ECS&) {
				pass.clearCommand(color, clearFlags);
			});
		}

	private:

		template<typename... Deps>
		Task& _task(Task&& t, Deps... deps) {
			entt::id_type taskHandle = mTasks.size();
			mBuilder.bind(taskHandle);
			_assignDeps(std::forward<Deps>(deps)...);

			mTasks.emplace_back(std::move(t));
			return mTasks.back();
		}

		template<typename Dep, typename... Deps>
		void _assignDeps(Dep dep, Deps... deps) {
			if constexpr (std::is_same_v<Dep, FramebufferHandle>) {
				mBuilder.rw(dep.mHandle);
			}
			else {
				static_assert(false, "dead");
			}
			if constexpr (sizeof...(Deps) > 0) {
				_assignDeps(deps...);
			}
		}

		std::vector<Task> mTasks;
		entt::flow mBuilder;
			
		FrameData mFrameData;
	};
}
