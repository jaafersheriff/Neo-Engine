#pragma once

#include "Util/Util.hpp"

#include <entt/resource/cache.hpp>
#include <string>
#include <memory>
#include <optional>

namespace neo {
	class ResourceManagers;

	constexpr HashedString::hash_type NEO_INVALID_HANDLE = 0;

	template<typename ResourceType>
	struct ResourceHandle {
		ResourceHandle()
			: mHandle(NEO_INVALID_HANDLE)
		{}
		ResourceHandle(entt::id_type handle)
			: mHandle(handle)
		{}
		ResourceHandle(HashedString id)
			: mHandle(id.value())
		{}

		entt::id_type mHandle;

		bool operator==(const ResourceHandle<ResourceType>& other) const noexcept {
			return mHandle == other.mHandle;
		}
	};

	template<typename ResourceType>
	struct BackedResource {
		template<typename... Args>
		BackedResource(Args... args)
			: mResource(std::forward<Args>(args)...)
		{}
		ResourceType mResource;
		std::optional<std::string> mDebugName;
	};

	template<typename DerivedManager, typename ResourceType, typename ResourceLoadDetails>
	class ResourceManagerInterface {
		friend ResourceManagers;
	public:

		bool isValid(ResourceHandle<ResourceType> id) const {
			return id.mHandle != NEO_INVALID_HANDLE && mCache.contains(id.mHandle);
		}

		bool isQueued(ResourceHandle<ResourceType> id) const {
			if (id.mHandle != NEO_INVALID_HANDLE) {
				return false;
			}
			// TODO - this is a linear search :(
			// But maybe it's fine because we shouldn't be queueing up a bunch of stuff every single frame..
			for (auto& res : mQueue) {
				if (id == res.mHandle) {
					return true;
				}
			}
			return false;
		}

		ResourceType& resolve(HashedString id) {
			return _resolveFinal(ResourceHandle<ResourceType>(id));
		}

		const ResourceType& resolve(HashedString id) const {
			return _resolveFinal(ResourceHandle<ResourceType>(id));
		}

		const ResourceType& resolve(ResourceHandle<ResourceType> id) const {
			return _resolveFinal(id);
		}

		ResourceType& resolve(ResourceHandle<ResourceType> id) {
			return _resolveFinal(id);
		}

		[[nodiscard]] ResourceHandle<ResourceType> asyncLoad(HashedString id, ResourceLoadDetails details) const {
			ResourceHandle<ResourceType> resourceId(id.value());
			if (isValid(resourceId) || isQueued(resourceId)) {
				return resourceId;
			}
			return static_cast<const DerivedManager*>(this)->_asyncLoadImpl(resourceId, details, std::string(id.data()));
		}

		[[nodiscard]] ResourceHandle<ResourceType> asyncLoad(ResourceHandle<ResourceType> id, ResourceLoadDetails details) const {
			if (isValid(id) || isQueued(id)) {
				return id;
			}
			return static_cast<const DerivedManager*>(this)->_asyncLoadImpl(id, details, std::nullopt);
		}

		void discard(ResourceHandle<ResourceType> id) const {
			// TODO - this should also be queued?
			if (isValid(id) || isQueued(id)) {
				NEO_UNUSED(id);
				NEO_LOG_E("Unsupported discard");
				//static_cast<DerivedManager*>(this)->_discardImpl(id);
			}
		}

	protected:
		struct ResourceLoadDetails_Internal {
			ResourceHandle<ResourceType> mHandle;
			ResourceLoadDetails mLoadDetails;
			std::optional<std::string> mDebugName;
		};

		void clear() {
			static_cast<DerivedManager*>(this)->_clearImpl();
		}

		void tick() {
			static_cast<DerivedManager*>(this)->_tickImpl();
		}
		mutable std::vector<ResourceLoadDetails_Internal> mQueue;
		entt::resource_cache<BackedResource<ResourceType>> mCache;
		std::shared_ptr<BackedResource<ResourceType>> mFallback;

	private:
		ResourceType& _resolveFinal(ResourceHandle<ResourceType> id) const {
			auto handle = mCache.handle(id.mHandle);
			if (handle) {
				return const_cast<ResourceType&>(handle.get().mResource);
			}
			NEO_LOG_W("Invalid resource requested! Did you check for validity?");
			return mFallback->mResource;
		}
	};
}
