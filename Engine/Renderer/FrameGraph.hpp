#pragma once

#include "ECS/ECS.hpp"
#include "ResourceManager/ResourceManagers.hpp"
#include "GLObjects/ResolvedShaderInstance.hpp"

#include <ext/entt_incl.hpp>

namespace neo {
	using Viewport = glm::uvec4;
	using UBO = std::map<const char*, ResolvedShaderInstance::UniformVariant>;

	class FrameGraph {
	public:
		using Task = std::function<void(const ResourceManagers& resourceManagers, const ECS& ecs)>;

		void execute(const ResourceManagers& resourceManagers, const ECS& ecs);

		template<typename... Deps>
		void task(Task&& t, Deps... deps) {
			entt::id_type taskHandle = reinterpret_cast<entt::id_type>(&t); // Stop it
			mBuilder.bind(taskHandle);
			mTasks.emplace(taskHandle, std::move(t));

			_assignDeps(deps...);
		}

		template<typename... Deps>
		void pass(FramebufferHandle target, Viewport viewport, Task t, Deps... deps) {
				NEO_LOG_V("Creating pass task");
			task(std::move([_t = std::move(t), target, viewport](const ResourceManagers& resourceManager, const ECS& ecs) {
				//NEO_ASSERT(resourceManager.mFramebufferManager.isValid(target), "Invalid target handle");
				NEO_LOG_V("Executing pass task");
				if (!resourceManager.mFramebufferManager.isValid(target)) {
					return;
				}
				glViewport(viewport.x, viewport.y, viewport.z, viewport.w);
				const auto& fb = resourceManager.mFramebufferManager.resolve(target);
				fb.bind();
				NEO_LOG_V("Executing draw func");
				_t(resourceManager, ecs);
			}), target, std::forward<Deps>(deps)...);
		}

		void clear(FramebufferHandle handle, glm::vec4 color, types::framebuffer::AttachmentBits clearFlags) {
			task(std::move([=](const ResourceManagers& resourceManager, const ECS&) {
				//NEO_ASSERT(resourceManager.mFramebufferManager.isValid(handle), "Invalid clear handle");
				if (!resourceManager.mFramebufferManager.isValid(handle)) {
					return;
				}
				const auto& fb = resourceManager.mFramebufferManager.resolve(handle);
				fb.bind();
				fb.clear(color, clearFlags);
			}), handle);
		}

	private:

		template<typename Dep, typename... Deps>
		void _assignDeps(Dep& dep, Deps... deps) {
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
	};
}
