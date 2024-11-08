#pragma once

#include "ResourceManager/ResourceManagers.hpp"

#include <ext/entt_incl.hpp>

namespace neo {

	class FrameGraph {
	public:
		using Task = std::function<void(const ResourceManagers& resourceManagers)>;

		template<typename... Deps>
		void task(Task t, Deps... deps) {
			entt::id_type taskHandle = reinterpret_cast<entt::id_type>(&t); // Stop it
			mBuilder.bind(taskHandle);
			mTasks.emplace(taskHandle, std::move(t));

			_assignDeps(deps...);
		}

		void clear(FramebufferHandle handle, glm::vec4 color, types::framebuffer::AttachmentBits clearFlags) {
			task([=](const ResourceManagers& resourceManager) {
				NEO_ASSERT(resourceManager.mFramebufferManager.isValid(handle), "Invalid clear handle");
				const auto& fb = resourceManager.mFramebufferManager.resolve(handle);
				fb.bind();
				fb.clear(color, clearFlags);
			}, handle);
		}

	private:

		template<typename Dep, typename... Deps>
		void _assignDeps(Dep& dep, Deps... deps) {
			if constexpr (std::is_same_v<Dep, FramebufferHandle>) {
				mBuilder.rw(dep.mHandle);
			}
			if constexpr (sizeof...(Deps) > 0) {
				_assignDeps(deps...);
			}
		}

		std::map<entt::id_type, Task> mTasks;
		entt::flow mBuilder;
	};
}
