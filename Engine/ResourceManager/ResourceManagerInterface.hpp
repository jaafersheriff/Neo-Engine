#pragma once

#include "Util/Util.hpp"

#include "ResourceManager/ResourceCache.hpp"
#include <string>
#include <memory>
#include <optional>
#include <mutex>
#include <chrono>

// The public face of the resource system
namespace neo {

	constexpr HashedString::hash_type NEO_INVALID_HANDLE = UINT32_MAX;

	// The stable identifier for a resource, valid for as long as the resource lives. Typed on
	// purpose: a TextureHandle cannot be handed to a mesh cache.
	template<typename ResourceType>
	struct ResourceHandle {
		ResourceHandle()
			: mHandle(NEO_INVALID_HANDLE)
		{}
		ResourceHandle(HashedString::hash_type handle)
			: mHandle(handle)
		{}
		ResourceHandle(HashedString id)
			: mHandle(id.value())
		{}

		HashedString::hash_type mHandle;

		bool operator==(const ResourceHandle<ResourceType>& other) const noexcept {
			return mHandle == other.mHandle;
		}
		bool operator!=(const ResourceHandle<ResourceType>& other) const noexcept {
			return !(mHandle == other.mHandle);
		}
		bool operator==(const HashedString::hash_type& other) const noexcept {
			return mHandle == other;
		}
		bool operator!=(const HashedString::hash_type& other) const noexcept {
			return !(mHandle == other);
		}
	};
}

// So a handle can key the cache's map. Handles are already hashed strings, nothing left to mix.
template<typename ResourceType>
struct std::hash<neo::ResourceHandle<ResourceType>> {
	size_t operator()(const neo::ResourceHandle<ResourceType>& handle) const noexcept {
		return static_cast<size_t>(handle.mHandle);
	}
};

namespace neo {
	class ResourceManagers;

	// RetainFrames > 0 keeps a resource alive for that many frames after the last resolve().
	template<typename DerivedManager, typename ResourceType, typename ResourceLoadDetails, uint8_t RetainFrames = 0>
	class ResourceManagerInterface {
		friend ResourceManagers;
	public:
		using Cache = ResourceCache<ResourceType, RetainFrames>;
		static constexpr bool kTracksEviction = Cache::kTracksEviction;

		bool isValid(const ResourceHandle<ResourceType>& id) const {
			return id != NEO_INVALID_HANDLE && mCache.contains(id);
		}

		bool isQueued(const ResourceHandle<ResourceType>& id) const {
			if (id == NEO_INVALID_HANDLE) {
				return false;
			}
			// TODO - this is a linear search :(
			// But maybe it's fine because we shouldn't be queueing up a bunch of stuff every single frame..
			{
				std::lock_guard<std::mutex> lock(mLoadQueueMutex);
				for (auto& res : mLoadQueue) {
					if (id == res.mHandle) {
						return true;
					}
				}
			}
			return false;
		}

		bool isDiscardQueued(const ResourceHandle<ResourceType>& id) const {
			if (id == NEO_INVALID_HANDLE) {
				return false;
			}
			// TODO - this is a linear search :(
			// But maybe it's fine because we shouldn't be queueing up a bunch of stuff every single frame..
			{
				std::lock_guard<std::mutex> lock(mDiscardQueueMutex);
				for (auto& res : mDiscardQueue) {
					if (id == res.mHandle) {
						return true;
					}
				}
			}
			return false;
		}

		ResourceType& resolve(HashedString id) {
			return resolve(ResourceHandle<ResourceType>(id));
		}

		const ResourceType& resolve(const HashedString& id) const {
			return resolve(ResourceHandle<ResourceType>(id));
		}

		const ResourceType& resolve(const ResourceHandle<ResourceType>& id) const {
			return _resolveFinal(id).mResource;
		}

		ResourceType& resolve(const ResourceHandle<ResourceType>& id) {
			return _resolveFinal(id).mResource;
		}

		uint64_t getTimeStamp(const HashedString& id) const {
			return getTimeStamp(ResourceHandle<ResourceType>(id));
		}

		uint64_t getTimeStamp(const ResourceHandle<ResourceType>& id) const {
			return _resolveFinal(id).mCreationTimeStamp;
		}

		[[nodiscard]] ResourceHandle<ResourceType> asyncLoad(HashedString id, ResourceLoadDetails details) const {
			return asyncLoad(ResourceHandle<ResourceType>(id.value()), details, std::string(id.data()));
		}

		[[nodiscard]] ResourceHandle<ResourceType> asyncLoad(ResourceHandle<ResourceType> id, ResourceLoadDetails details, std::optional<std::string> debugName = std::nullopt) const {
			if (!isDiscardQueued(id) && (isValid(id) || isQueued(id))) {
				return id;
			}
			return static_cast<const DerivedManager*>(this)->_asyncLoadImpl(id, details, debugName);
		}

		void transact(ResourceHandle<ResourceType> handle, std::function<void(ResourceType&)> transaction) const {
			std::lock_guard<std::mutex> lock(mTransactionQueueMutex);
			mTransactionQueue.emplace_back(std::make_pair(handle, transaction));
		}

		void discard(ResourceHandle<ResourceType> id) const {
			if (!isDiscardQueued(id) && (isValid(id) || isQueued(id))) {
				std::lock_guard<std::mutex> lock(mDiscardQueueMutex);
				mDiscardQueue.emplace_back(id);
			}
		}

	protected:
		struct ResourceLoadDetails_Internal {
			ResourceHandle<ResourceType> mHandle;
			ResourceLoadDetails mLoadDetails;
			std::optional<std::string> mDebugName;
		};

		void clear() {
			{
				std::lock_guard<std::mutex> lock(mLoadQueueMutex);
				mLoadQueue.clear();
			}
			{
				std::lock_guard<std::mutex> lock(mDiscardQueueMutex);
				mDiscardQueue.clear();
			}
			{
				std::lock_guard<std::mutex> lock(mTransactionQueueMutex);
				mTransactionQueue.clear();
			}
			mCache.forEach([this](CachedResource<ResourceType>& entry) {
				static_cast<DerivedManager*>(this)->_destroyImpl(entry);
			});
			mCache.clear();

			if (mFallback) {
				static_cast<DerivedManager*>(this)->_destroyImpl(*mFallback);
				mFallback.reset();
			}
		}

		void init() {
			NEO_ASSERT(!mFallback, "Fallback resource already created");
			static_cast<DerivedManager*>(this)->_initImpl();
		}

		void tick() {
			if constexpr (kTracksEviction) {
				mCache.age([this](CachedResource<ResourceType>& entry) {
					static_cast<DerivedManager*>(this)->_destroyImpl(entry);
				});
			}
			static_cast<DerivedManager*>(this)->_tickImpl();
		}
		mutable std::mutex mLoadQueueMutex;
		mutable std::vector<ResourceLoadDetails_Internal> mLoadQueue;

		mutable std::mutex mDiscardQueueMutex;
		mutable std::vector<ResourceHandle<ResourceType>> mDiscardQueue;

		mutable std::mutex mTransactionQueueMutex;
		mutable std::vector<std::pair<ResourceHandle<ResourceType>, std::function<void(ResourceType&)>>> mTransactionQueue;

		Cache mCache;
		std::shared_ptr<CachedResource<ResourceType>> mFallback;

	private:
		CachedResource<ResourceType>& _resolveFinal(const ResourceHandle<ResourceType>& id) const {
			if (const auto* resource = mCache.resolve(id)) {
				return const_cast<CachedResource<ResourceType>&>(*resource);
			}
			NEO_FAIL("Invalid resource requested! Did you check for validity?");
			return *mFallback;
		}
	};
}
