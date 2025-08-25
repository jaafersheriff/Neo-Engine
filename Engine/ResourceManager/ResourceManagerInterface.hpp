#pragma once

#include "Util/Util.hpp"

#include <entt/resource/cache.hpp>
#include <string>
#include <memory>
#include <optional>
#include <mutex>
#include <chrono>

namespace neo {
	namespace {
		uint64_t getCurrentTimestamp() {
			using namespace std::chrono;
			return duration_cast<microseconds>(steady_clock::now().time_since_epoch()).count();
		}
	}

	class ResourceManagers;

	constexpr HashedString::hash_type NEO_INVALID_HANDLE = 65535;

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
		bool operator==(const entt::id_type& other) const noexcept {
			return mHandle == other;
		}
		bool operator!=(const entt::id_type& other) const noexcept {
			return !(mHandle == other);
		}

	};

	template<typename ResourceType>
	struct BackedResource {
		template<typename... Args>
		BackedResource(Args... args)
			: mResource(std::forward<Args>(args)...)
			, mCreationTimeStamp(getCurrentTimestamp())
		{}
		ResourceType mResource;
		std::optional<std::string> mDebugName;
		const uint64_t mCreationTimeStamp;
	};

	template<typename DerivedManager, typename ResourceType, typename ResourceLoadDetails>
	class ResourceManagerInterface {
		friend ResourceManagers;
	public:

		bool isValid(const ResourceHandle<ResourceType>& id) const {
			return id != NEO_INVALID_HANDLE && mCache.contains(id.mHandle);
		}

		bool isQueued(const ResourceHandle<ResourceType>& id) const {
			if (id == NEO_INVALID_HANDLE) {
				return false;
			}
			// TODO - this is a linear search :(
			// But maybe it's fine because we shouldn't be queueing up a bunch of stuff every single frame..
			{
				std::shared_lock<std::mutex> lock(mLoadQueueMutex);
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
				std::shared_lock<std::mutex> lock(mDiscardQueueMutex);
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
			mCache.each([this](BackedResource<ResourceType>& resource) {
				static_cast<DerivedManager*>(this)->_destroyImpl(resource);
			});
			mCache.clear();
		}

		void tick() {
			static_cast<DerivedManager*>(this)->_tickImpl();
		}
		mutable std::mutex mLoadQueueMutex;
		mutable std::vector<ResourceLoadDetails_Internal> mLoadQueue;

		mutable std::mutex mDiscardQueueMutex;
		mutable std::vector<ResourceHandle<ResourceType>> mDiscardQueue;

		mutable std::mutex mTransactionQueueMutex;
		mutable std::vector<std::pair<ResourceHandle<ResourceType>, std::function<void(ResourceType&)>>> mTransactionQueue;

		entt::resource_cache<BackedResource<ResourceType>> mCache;
		std::shared_ptr<BackedResource<ResourceType>> mFallback;

	private:
		BackedResource<ResourceType>& _resolveFinal(const ResourceHandle<ResourceType>& id) const {
			auto handle = mCache.handle(id.mHandle);
			if (handle) {
				return const_cast<BackedResource<ResourceType>&>(handle.get());
			}
			NEO_FAIL("Invalid resource requested! Did you check for validity?");
			return *mFallback;
		}
	};
}
