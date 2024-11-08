#pragma once

#include "ResourceManager/ResourceManagers.hpp"

namespace neo {

	class FrameGraph {
	public:
		using Task = std::function<void(const ResourceManagers& resourceManagers)>;

		class ReadWriteResource {
		};
		class ReadOnlyResource {
		};

		template<typename... Deps>
		void task(Task t) {
			entt::id_type handle = reinterpret_cast<entt::id_type>(&t); // Stop it
			mBuilder.bind(handle);
			mTasks.emplace(handle, std::move(t));
		}

		void clear(FramebufferHandle handle, glm::vec4 color, types::framebuffer::AttachmentBits clearFlags) {
			task([=](const ResourceManagers& resourceManager) {
				NEO_ASSERT(resourceManager.mFramebufferManager.isValid(handle), "Invalid clear handle");
				resourceManager.mFramebufferManager.resolve(handle).clear(color, clearFlags);
			});
			_assignDeps(handle);
		}

	private:

		template<typename Dep, typename... Deps>
		void _assignDeps(Dep& dep, Deps... deps) {
			if constexpr (std::is_same_v<Dep, FramebufferHandle>) {
				mBuilder.rw(dep.mHandle);
			}
		}

		std::map<entt::id_type, Task> mTasks;
		entt::flow mBuilder;
	};
}