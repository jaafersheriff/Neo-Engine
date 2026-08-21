#pragma once

#include "Util/Util.hpp"

#include "ResourceManager/ResourceCache.hpp"
#include <string>
#include <memory>
#include <optional>
#include <mutex>
#include <chrono>
#include <unordered_set>

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

		// True from the moment asyncLoad claims the handle until the tick that publishes it has finished
		// with it - which deliberately includes the window where it has left the load queue but is not
		// yet in the cache. That window is what made a concurrent asyncLoad enqueue the same resource
		// twice.
		bool isQueued(const ResourceHandle<ResourceType>& id) const {
			if (id == NEO_INVALID_HANDLE) {
				return false;
			}
			std::lock_guard<std::mutex> lock(mPendingMutex);
			return mPending.find(id) != mPending.end();
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

		// Claiming the handle and enqueueing it are one step, which is what makes this safe to call for
		// the same resource from several threads at once. It used to check "already valid or queued?"
		// and then enqueue under a different lock, so two callers could both find nothing and both
		// enqueue - loading the resource twice, and leaking the first copy when the second insert
		// replaced it in the cache. Serial callers never saw it; going wide over glTF nodes would have.
		[[nodiscard]] ResourceHandle<ResourceType> asyncLoad(ResourceHandle<ResourceType> id, ResourceLoadDetails details, std::optional<std::string> debugName = std::nullopt) const {
			// A discard already in flight means the caller wants this reloaded, so it skips the claim
			// entirely and always enqueues - same behaviour as before.
			if (!isDiscardQueued(id)) {
				if (isValid(id)) {
					return id;
				}
				std::lock_guard<std::mutex> lock(mPendingMutex);
				if (!mPending.insert(id).second) {
					// Somebody else got here first. They will enqueue it; we are done.
					return id;
				}
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
		// Every _tickImpl must call this for each entry it takes off the load queue, once it has been
		// published to the cache or has failed - and not before. Until it runs, asyncLoad treats the
		// handle as still claimed, which is what closes the gap between leaving the queue and landing
		// in the cache. Miss it and the resource can never be loaded again.
		void _finishPending(const ResourceHandle<ResourceType>& id) const {
			std::lock_guard<std::mutex> lock(mPendingMutex);
			mPending.erase(id);
		}

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
				std::lock_guard<std::mutex> lock(mPendingMutex);
				mPending.clear();
			}
			{
				std::lock_guard<std::mutex> lock(mDiscardQueueMutex);
				mDiscardQueue.clear();
			}
			{
				std::lock_guard<std::mutex> lock(mTransactionQueueMutex);
				mTransactionQueue.clear();
			}
			// Anything still waiting in the graveyards goes now - clear() is a quiesced teardown, so
			// there is nobody left who could be holding a pointer.
			for (std::unique_ptr<CachedResource<ResourceType>>& retired : mDoomed) {
				static_cast<DerivedManager*>(this)->_destroyImpl(*retired);
			}
			mDoomed.clear();
			for (std::unique_ptr<CachedResource<ResourceType>>& retired : mRetired) {
				static_cast<DerivedManager*>(this)->_destroyImpl(*retired);
			}
			mRetired.clear();

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

		// Retires a resource: unpublished from the cache immediately, destroyed two ticks later.
		//
		// The two halves matter separately. Unpublishing under the cache's exclusive lock means the
		// main thread can never resolve a handle that is already doomed. Deferring the destruction
		// means a pointer it resolved *before* that has a full frame to fall out of scope. Together
		// they are what let _tick run concurrently with the main thread at all - without them, tick
		// can only run while main is blocked, which costs main the whole duration of the tick.
		void retire(const ResourceHandle<ResourceType>& id) {
			if (std::unique_ptr<CachedResource<ResourceType>> retired = mCache.extract(id)) {
				mRetired.emplace_back(std::move(retired));
			}
		}

		void tick() {
			// Anything retired two ticks ago is now unreachable by everyone, so it can go.
			for (std::unique_ptr<CachedResource<ResourceType>>& doomed : mDoomed) {
				static_cast<DerivedManager*>(this)->_destroyImpl(*doomed);
			}
			mDoomed.clear();
			std::swap(mDoomed, mRetired);

			if constexpr (kTracksEviction) {
				// Eviction retires through the same path as an explicit discard.
				mExpiredScratch.clear();
				mCache.age([this](const ResourceHandle<ResourceType>& id) {
					mExpiredScratch.emplace_back(id);
				});
				for (const ResourceHandle<ResourceType>& id : mExpiredScratch) {
					retire(id);
				}
			}

			static_cast<DerivedManager*>(this)->_tickImpl();
		}
		mutable std::mutex mLoadQueueMutex;
		mutable std::vector<ResourceLoadDetails_Internal> mLoadQueue;

		// Handles claimed by asyncLoad and not yet finished with by a tick. Guarded by its own mutex so
		// the claim never has to be taken while holding the load queue, and a set rather than a scan of
		// mLoadQueue because ImGui asks isQueued every frame - behind a large scene load that was a
		// linear search over thousands of pending entries, under a lock the loader was contending for.
		mutable std::mutex mPendingMutex;
		mutable std::unordered_set<ResourceHandle<ResourceType>> mPending;

		mutable std::mutex mDiscardQueueMutex;
		mutable std::vector<ResourceHandle<ResourceType>> mDiscardQueue;

		mutable std::mutex mTransactionQueueMutex;
		mutable std::vector<std::pair<ResourceHandle<ResourceType>, std::function<void(ResourceType&)>>> mTransactionQueue;

		Cache mCache;
		std::shared_ptr<CachedResource<ResourceType>> mFallback;

		// The graveyard. mRetired collects this tick's retirements; mDoomed holds the previous
		// tick's and is destroyed at the start of the next one, giving every retired resource at
		// least a full frame between being unpublished and being freed.
		std::vector<std::unique_ptr<CachedResource<ResourceType>>> mRetired;
		std::vector<std::unique_ptr<CachedResource<ResourceType>>> mDoomed;
		std::vector<ResourceHandle<ResourceType>> mExpiredScratch;

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
