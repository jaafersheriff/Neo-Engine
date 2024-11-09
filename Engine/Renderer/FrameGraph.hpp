#pragma once

#include "ECS/ECS.hpp"
#include "ResourceManager/ResourceManagers.hpp"
#include "GLObjects/ResolvedShaderInstance.hpp"

#include <ext/entt_incl.hpp>

namespace neo {
	using Viewport = glm::uvec4;

	struct UBO {

		void bindUniform(const char* name, const ResolvedShaderInstance::UniformVariant& variant) {
			mUniforms[name] = variant;
		}

		void bindTexture(const char* name, TextureHandle handle) {
			mTextures[name] = handle;
		}

		std::map<const char*, ResolvedShaderInstance::UniformVariant> mUniforms;
		std::map<const char*, TextureHandle> mTextures;
	};
	struct ShaderDefinesFG {
		ShaderDefinesFG() = default;

		void set(const ShaderDefine& define) {
			mDefines[define] = true;
		}

		std::map<ShaderDefine, bool> mDefines;
	};


	using Command = uint64_t;
	enum class CommandType : uint8_t {
		StartPass = 0,
		Clear = 1,
		Draw = 2
	};

	struct Pass {
		Pass(uint8_t fbID, uint8_t vpID, uint8_t shaderID, UBO& ubo)
			: mFramebufferIndex(fbID)
			, mViewportIndex(vpID)
			, mShaderIndex(shaderID)
			, mPassUBO(ubo)
		{}

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

		void drawCommand(const MeshHandle& mesh, const UBO& ubo, const ShaderDefines& drawDefines) {
			uint64_t meshIndex = mMeshIndex;
			mMeshes[mMeshIndex++] = mesh;
			uint64_t uboIndex = mUBOIndex;
			mUBOs[mUBOIndex++] = ubo;
			uint64_t definesIndex = mShaderDefinesIndex;
			mShaderDefines[mShaderDefinesIndex++].mDefines = drawDefines.mDefines;

			Command& command = mCommands[mCommandIndex++];
			command = 0
				| static_cast<uint64_t>(CommandType::Draw) << (64 - 3)
				| meshIndex << (64 - 3 - 10)
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

		UBO& createUBO() {
			return mUBOs[mUBOIndex++];
		}
		void bindUniform(const char* name, const ResolvedShaderInstance::UniformVariant& variant) {
			mPassUBO.mUniforms[name] = variant;
		}
		void bindTexture(const char* name, TextureHandle handle) {
			mPassUBO.mTextures[name] = handle;
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
		uint8_t mFramebufferIndex = 0;
		uint8_t mViewportIndex = 0;
		uint8_t mShaderIndex = 0;
		UBO mPassUBO;
		ShaderDefines mPassDefines;

		Command mCommands[1024];
		uint16_t mCommandIndex = 0;

		ClearParams mClearParams[8];
		uint8_t mClearParamsIndex = 0; // 3 bits

		ShaderDefinesFG mShaderDefines[1024];
		uint16_t mShaderDefinesIndex = 0; // 10 bits

		UBO mUBOs[1024];
		uint16_t mUBOIndex = 0; // 10 bits

		MeshHandle mMeshes[1024];
		uint16_t mMeshIndex = 0; // 10 bits
	};


	struct PassQueue {

		uint16_t addPass(FramebufferHandle handle, Viewport vp, UBO& ubo, ShaderHandle shaderHandle = {}) {
			mFramebufferHandles[mFramebufferHandleIndex] = handle;
			mViewports[mViewportIndex] = vp;
			mShaderHandles[mShaderHandleIndex] = shaderHandle;

			mCommandQueues.emplace_back(mFramebufferHandleIndex++, mViewportIndex++, mShaderHandleIndex++, ubo);
			return static_cast<uint16_t>(mCommandQueues.size() - 1);
		}

		Pass& getPass(uint16_t index) {
			return mCommandQueues[index];
		}

		FramebufferHandle mFramebufferHandles[256];
		uint8_t mFramebufferHandleIndex = 0;

		Viewport mViewports[256];
		uint8_t mViewportIndex = 0;

		ShaderHandle mShaderHandles[256];
		uint8_t mShaderHandleIndex = 0;

	private:
		std::vector<Pass> mCommandQueues;
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
		};

		void execute(const ResourceManagers& resourceManagers, const ECS& ecs);

		template<typename... Deps>
		void pass(FramebufferHandle target, Viewport viewport, ShaderHandle shader, Task::Functor t, Deps... deps) {
			UBO emptyUBO;
			uint16_t passIndex = mPassQueue.addPass(target, viewport, emptyUBO, shader);
			_task(std::move(Task(passIndex, [_t = std::move(t)](Pass& pass, const ResourceManagers& resourceManager, const ECS& ecs) {
				pass.startCommand();
				_t(pass, resourceManager, ecs);
			})), target, std::forward<Deps>(deps)...);
		}

		void clear(FramebufferHandle handle, glm::vec4 color, types::framebuffer::AttachmentBits clearFlags) {
			pass(handle, {}, {}, [=](Pass& pass, const ResourceManagers&, const ECS&) {
				pass.clearCommand(color, clearFlags);
			});
		}

	private:
		// TODO - these should go elsewhere
		void _startPass(const Command& command, const ResourceManagers& resourceManagers);
		void _clear(Pass& pass, Command& command);
		void _draw(Pass& pass, Command& command, const ResourceManagers& resourceManagers);

		template<typename... Deps>
		void _task(Task&& t, Deps... deps) {
			entt::id_type taskHandle = reinterpret_cast<entt::id_type>(&t); // Stop it
			mBuilder.bind(taskHandle);
			_assignDeps(std::forward<Deps>(deps)...);

			mTasks.emplace(taskHandle, std::move(t));
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

		std::map<entt::id_type, Task> mTasks;
		entt::flow mBuilder;
			
		PassQueue mPassQueue;
	};
}
