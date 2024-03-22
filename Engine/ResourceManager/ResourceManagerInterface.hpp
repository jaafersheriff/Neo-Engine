#pragma once

#include "Util/Util.hpp"

#include <entt/resource/cache.hpp>
#include <memory>

namespace neo {
	class ResourceManagers;

	template<typename DerivedManager, typename ResourceHandle, typename ResourceType, typename ResourceDetails>
	class ResourceManagerInterface {
		friend ResourceManagers;
	public:

		bool isValid(ResourceHandle id) const {
			return mCache.contains(id);
		}

		ResourceType& get(HashedString id) {
			return _getFinal(id);
		}

		const ResourceType& get(HashedString id) const {
			return _getFinal(id);
		}

		const ResourceType& get(ResourceHandle id) const {
			return _getFinal(id);
		}

		ResourceType& get(ResourceHandle id) {
			return _getFinal(id);
		}

		[[nodiscard]] ResourceHandle asyncLoad(HashedString id, ResourceDetails& details) const {
			return static_cast<const DerivedManager*>(this)->_asyncLoadImpl(id, details);
		}

	protected:
		void clear() {
			static_cast<DerivedManager*>(this)->_clearImpl();
		}

		void tick() {
			static_cast<DerivedManager*>(this)->_tickImpl();
		}
		mutable std::map<ResourceHandle, ResourceDetails> mQueue;
		entt::resource_cache<ResourceType> mCache;
		std::shared_ptr<ResourceType> mFallback;

	private:
		ResourceType& _getFinal(ResourceHandle id) const {
			// TODO - this (and all other resource managers) are iterating through the dense map twice here
			if (isValid(id)) {
				return const_cast<ResourceType&>(mCache.handle(id).get());
			}
			NEO_FAIL("Invalid resource requested");
			return *mFallback;
		}
	};
}
