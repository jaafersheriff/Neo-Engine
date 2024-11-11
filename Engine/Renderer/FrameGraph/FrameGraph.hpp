#pragma once

#include "ECS/ECS.hpp"
#include "ResourceManager/ResourceManagers.hpp"
#include "Renderer/GLObjects/PassState.hpp"
#include "Renderer/GLObjects/UniformBuffer.hpp"
#include "Renderer/FrameGraph/ShaderDefinesFG.hpp"
#include "Renderer/FrameGraph/FrameData.hpp"

#include <ext/entt_incl.hpp>

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
			, mPassState(state)
		{
		}

		~Pass() {
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
			mPassDefines.set(define);
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
		
	//private:
		FrameData& mFrameData;

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

		struct Draw {
			MeshHandle mMeshHandle;
			uint16_t mElementCount = 0;
			uint16_t mElementBufferOffset = 0;
		};
		Draw mDraws[1024];
		uint16_t mDrawIndex = 0; // 10 bits
	};

	class FrameGraph {
	public:
		FrameGraph(const ResourceManagers& rm)
			: mResourceManagers(rm)
		{}

		struct Task {
			friend FrameGraph;

			using Functor = std::function<void(Pass&, const ResourceManagers&, const ECS&)>;
			Task(entt::flow& builder)
				: mPassIndex(UINT16_MAX)
				, mBuilder(builder)
			{}
			Task(uint16_t passIndex, entt::flow& builder)
				: mPassIndex(passIndex)
				, mBuilder(builder)
			{}

			Task& with(Functor _f) {
				mPassBuilder = [f = std::move(_f)](Pass& pass, const ResourceManagers& resourceManagers, const ECS& ecs) {
					pass.startCommand();
					f(pass, resourceManagers, ecs);
				};
				return *this;
			}

			template<typename... Deps>
			Task& dependsOn(const ResourceManagers& resourceManagers, Deps... deps) {
				if constexpr (sizeof...(Deps) > 0) {
					_assignDeps(resourceManagers, std::forward<Deps>(deps)...);
				}

				return *this;
			}

			Task& setDebugName(const char* name) {
				mDebugName = std::string(name);
				return *this;
			}

		private:
			uint16_t mPassIndex;
			std::optional<Functor> mPassBuilder;
			std::optional<std::string> mDebugName;
			entt::flow& mBuilder;

			template<typename Dep, typename... Deps>
			void _assignDeps(const ResourceManagers& resourceManagers, const Dep& dep, Deps... deps) {
				if (dep == NEO_INVALID_HANDLE) {
					return;
				}
				if constexpr (std::is_same_v<Dep, FramebufferHandle>) {
					mBuilder.rw(dep.mHandle);
					if (resourceManagers.mFramebufferManager.isValid(dep)) {
						const Framebuffer& fb = resourceManagers.mFramebufferManager.resolve(dep);
						for (const TextureHandle& tex : fb.mTextures) {
							mBuilder.rw(tex.mHandle);
						}
					}
				}
				else if constexpr (std::is_same_v<Dep, TextureHandle>) {
					mBuilder.ro(dep.mHandle);
				}
				else {
					static_assert(false, "dead");
				}
				if constexpr (sizeof...(Deps) > 0) {
					return _assignDeps(resourceManagers, std::forward<Deps>(deps)...);
				}
			}
		};

		void execute(const ResourceManagers& resourceManagers, const ECS& ecs);

		Task& pass(FramebufferHandle target, Viewport viewport, Viewport scissor, PassState state, ShaderHandle shader) {
			uint16_t passIndex = mFrameData.addPass(target, viewport, scissor, state, shader);
			return _task(std::move(Task(passIndex, mBuilder)));
		}

		Task& clear(FramebufferHandle handle, glm::vec4 color, types::framebuffer::AttachmentBits clearFlags) {
			return pass(handle, {}, {}, {}, {})
				.with([=](Pass& pass, const ResourceManagers&, const ECS&) {
					pass.clearCommand(color, clearFlags);
				})
				.dependsOn(mResourceManagers, handle)
				.setDebugName("Clear")
			;
		}

	private:
		const ResourceManagers& mResourceManagers;

		Task& _task(Task&& t) {
			entt::id_type taskHandle = mTasks.size();
			mBuilder.bind(taskHandle);

			mTasks.emplace_back(std::move(t));
			return mTasks.back();
		}


		std::vector<Task> mTasks;
		entt::flow mBuilder;
			
		FrameData mFrameData;
	};
}
