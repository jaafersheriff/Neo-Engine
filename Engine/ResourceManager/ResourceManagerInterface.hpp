#pragma once

#include "Util/Util.hpp"

#include "Jobs/JobSystem.hpp"
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

	// The stable identifier for a resource, valid for as long as the resource lives
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

		[[nodiscard]] ResourceHandle<ResourceType> asyncLoad(ResourceHandle<ResourceType> id, ResourceLoadDetails details, std::optional<std::string> debugName = std::nullopt) const {
			// A discard already in flight means the caller wants this reloaded
			if (!isDiscardQueued(id)) {
				if (isValid(id)) {
					return id;
				}
				std::lock_guard<std::mutex> lock(mPendingMutex);
				if (!mPending.insert(id).second) {
					// Somebody else got here first
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

		// Takes a resource out of the cache and destroys it, here and now.
		//
		// This used to defer the destruction by two ticks behind a pair of graveyard vectors, because
		// the main thread could be holding a pointer it resolved just before the resource was
		// unpublished, and no lock takes an already-returned pointer back. That is no longer possible:
		// resolve() asserts it is on the render thread, so the render thread is the only one that can
		// ever hold a resolved pointer - and this runs on that same thread, from tick(), with every
		// pass of the previous frame returned. There is nobody left to wait for.
		//
		// The extract still has to happen under the cache's exclusive lock, because the ImGui panels
		// walk the cache from main under the shared one. The destruction then happens outside it,
		// since the entry is ours alone by that point.
		void _destroyNow(const ResourceHandle<ResourceType>& id) {
			if (std::unique_ptr<CachedResource<ResourceType>> destroyed = mCache.extract(id)) {
				static_cast<DerivedManager*>(this)->_destroyImpl(*destroyed);
			}
		}

		void tick() {
			if constexpr (kTracksEviction) {
				// Collected before destroying rather than destroyed inside the sweep: age() holds the
				// cache lock shared and the extract below needs it exclusively. Local rather than a
				// member - only the two evicting managers instantiate this, and an expiry is rare
				// enough that the vector usually never allocates.
				std::vector<ResourceHandle<ResourceType>> expired;
				mCache.age([&expired](const ResourceHandle<ResourceType>& id) {
					expired.emplace_back(id);
				});

				// Eviction and explicit discard are the same operation now, so they share the path.
				for (const ResourceHandle<ResourceType>& id : expired) {
					_destroyNow(id);
				}
			}

			static_cast<DerivedManager*>(this)->_tickImpl();
		}
		mutable std::mutex mLoadQueueMutex;
		mutable std::vector<ResourceLoadDetails_Internal> mLoadQueue;

		mutable std::mutex mPendingMutex;
		mutable std::unordered_set<ResourceHandle<ResourceType>> mPending;

		mutable std::mutex mDiscardQueueMutex;
		mutable std::vector<ResourceHandle<ResourceType>> mDiscardQueue;

		mutable std::mutex mTransactionQueueMutex;
		mutable std::vector<std::pair<ResourceHandle<ResourceType>, std::function<void(ResourceType&)>>> mTransactionQueue;

		Cache mCache;
		std::shared_ptr<CachedResource<ResourceType>> mFallback;

	private:
		CachedResource<ResourceType>& _resolveFinal(const ResourceHandle<ResourceType>& id) const {
			// The whole of resolve() and getTimeStamp() funnels through here, so this is the one place
			// the rule needs stating: a resolved resource is a live GL object, and GL belongs to the
			// thread that owns the context. Anything elsewhere that only needs a resource's shape
			// should be asking a manager for a descriptor instead - see TextureManager::getDescriptor.
			//
			// This is also what the cache's deferred destruction rests on. If the render thread is the
			// only thread that can be holding a resolved pointer, then it is safe for that same thread
			// to destroy one between frames, when every pass has returned.
			NEO_ASSERT(isRenderThread(), "Resource resolved off the render thread - resolve() hands out a live GL object");

			if (const auto* resource = mCache.resolve(id)) {
				return const_cast<CachedResource<ResourceType>&>(*resource);
			}
			NEO_FAIL("Invalid resource requested! Did you check for validity?");
			return *mFallback;
		}
	};
}
