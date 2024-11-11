#pragma once

#include "ECS/ECS.hpp"
#include "ResourceManager/ResourceManagers.hpp"
#include "Renderer/GLObjects/PassState.hpp"
#include "Renderer/GLObjects/UniformBuffer.hpp"
#include "Renderer/FrameGraph/ShaderDefinesFG.hpp"
#include "Renderer/FrameGraph/FrameData.hpp"
#include "Renderer/FrameGraph/Pass.hpp"

#include <ext/entt_incl.hpp>

namespace neo {

	class FrameGraph {
	public:
		FrameGraph(const ResourceManagers& rm, FrameData& frameData)
			: mResourceManagers(rm)
			, mFrameData(frameData)
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

		Task& pass(const FramebufferHandle& target, Viewport viewport, Viewport scissor, PassState state, ShaderHandle shader) {
			uint16_t passIndex = mFrameData.addPass(target, viewport, scissor, state, shader);
			return _task(std::move(Task(passIndex, mBuilder))).dependsOn(mResourceManagers, target);
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
		FrameData& mFrameData;

		Task& _task(Task&& t) {
			entt::id_type taskHandle = mTasks.size();
			mBuilder.bind(taskHandle);

			mTasks.emplace_back(std::move(t));
			return mTasks.back();
		}


		std::vector<Task> mTasks;
		entt::flow mBuilder;
			
	};
}
