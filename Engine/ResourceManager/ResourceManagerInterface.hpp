#pragma once

#include "Util/Util.hpp"

#include <entt/resource/cache.hpp>
#include <memory>

namespace neo {
	class ResourceManagers;

	template<typename DerivedManager, typename ResourceHandle, typename ResourceType, typename ResourceLoadDetails>
	class ResourceManagerInterface {
		friend ResourceManagers;
	public:

		bool isValid(ResourceHandle id) const {
			return mCache.contains(id);
		}

		bool isQueued(ResourceHandle id) const {
			return mQueue.find(id) != mQueue.end();
		}

		ResourceType& resolve(HashedString id) {
			return _resolveFinal(id);
		}

		const ResourceType& resolve(HashedString id) const {
			return _resolveFinal(id);
		}

		const ResourceType& resolve(ResourceHandle id) const {
			return _resolveFinal(id);
		}

		ResourceType& resolve(ResourceHandle id) {
			return _resolveFinal(id);
		}

		[[nodiscard]] ResourceHandle asyncLoad(HashedString id, ResourceLoadDetails details) const {
			if (isValid(id) || isQueued(id)) {
				return id;
			}
			return static_cast<const DerivedManager*>(this)->_asyncLoadImpl(id, details);
		}

	protected:
		void clear() {
			static_cast<DerivedManager*>(this)->_clearImpl();
		}

		void tick() {
			static_cast<DerivedManager*>(this)->_tickImpl();
		}
		mutable std::map<ResourceHandle, ResourceLoadDetails> mQueue;
		entt::resource_cache<ResourceType> mCache;
		std::shared_ptr<ResourceType> mFallback;

	private:
		ResourceType& _resolveFinal(ResourceHandle id) const {
			auto handle = mCache.handle(id);
			if (handle) {
				return const_cast<ResourceType&>(handle.get());
			}
			NEO_LOG_W("Invalid resource requested! Did you check for validity?");
			return *mFallback;
		}
	};
}
