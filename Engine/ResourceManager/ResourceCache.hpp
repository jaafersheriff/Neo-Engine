#pragma once

#include <entt/container/dense_map.hpp>

#include <chrono>
#include <cstdint>
#include <memory>
#include <optional>
#include <shared_mutex>
#include <string>
#include <utility>
#include <vector>

namespace neo {

	// Declared here, defined by owning ResourceManagerInterface derivative
	template<typename ResourceType>
	struct ResourceHandle;

	inline uint64_t getCurrentTimestamp() {
		using namespace std::chrono;
		return duration_cast<microseconds>(steady_clock::now().time_since_epoch()).count();
	}

	template<typename ResourceType>
	struct CachedResource {
		template<typename... Args>
		CachedResource(Args... args)
			: mResource(std::forward<Args>(args)...)
			, mCreationTimeStamp(getCurrentTimestamp())
		{}

		ResourceHandle<ResourceType> mHandle;
		ResourceType mResource; // GL-wrapper object
		std::optional<std::string> mDebugName;
		const uint64_t mCreationTimeStamp;
	};

	// Handle-keyed storage for one resource type, with an optional eviction policy.
	// RetainFrames used for pooling. 0 means nothing is ever evicted unless erase() is called
	template<typename ResourceType, uint8_t RetainFrames = 0>
	class ResourceCache {
		// Dense position of an entry
		using EntryPosition = uint32_t;
		static constexpr EntryPosition kInvalidEntryPosition = UINT32_MAX;

	public:
		using Handle = ResourceHandle<ResourceType>;
		using Entry = CachedResource<ResourceType>;
		static constexpr bool kTracksEviction = RetainFrames > 0;


		// Deliberately does NOT count as a use
		template<typename Func>
		void forEach(Func&& func) const {
			std::shared_lock<std::shared_mutex> lock(mMutex);
			for (const std::unique_ptr<Entry>& entry : mEntries) {
				func(*entry);
			}
		}
		template<typename Func>
		void forEach(Func&& func) {
			std::shared_lock<std::shared_mutex> lock(mMutex);
			for (const std::unique_ptr<Entry>& entry : mEntries) {
				func(*entry);
			}
		}

		[[nodiscard]] bool contains(Handle handle) const {
			std::shared_lock<std::shared_mutex> lock(mMutex);
			return mEntryPositions.contains(handle);
		}

		[[nodiscard]] size_t size() const {
			std::shared_lock<std::shared_mutex> lock(mMutex);
			return mEntries.size();
		}

		// The only path that counts as "still in use".
		[[nodiscard]] const Entry* resolve(Handle handle) const {
			// TODO - An evicting cache writes the counter here, so it needs the lock exclusively, not great for perf
			std::conditional_t<kTracksEviction, std::unique_lock<std::shared_mutex>, std::shared_lock<std::shared_mutex>> lock(mMutex);
			const EntryPosition entryPos = _getEntryPosition(handle);
			if (entryPos == kInvalidEntryPosition) {
				return nullptr;
			}
			_markUsed(entryPos);
			return mEntries[entryPos].get();
		}

		[[nodiscard]] Entry* resolve(Handle handle) {
			return const_cast<Entry*>(std::as_const(*this).resolve(handle));
		}

		void insert(Handle handle, Entry&& entry) {
			std::unique_lock<std::shared_mutex> lock(mMutex);
			entry.mHandle = handle;

			EntryPosition entryPos = _getEntryPosition(handle);
			if (entryPos != kInvalidEntryPosition) {
				// Replace
				mEntries[entryPos] = std::make_unique<Entry>(std::move(entry));
				_markUsed(entryPos);
				return;
			}

			// Emplace
			mEntryPositions.insert_or_assign(handle, static_cast<EntryPosition>(mEntries.size()));
			mEntries.emplace_back(std::make_unique<Entry>(std::move(entry)));
			if constexpr (kTracksEviction) {
				mFramesUntilEviction.emplace_back(RetainFrames);
			}
		}

		void erase(Handle handle) {
			std::unique_lock<std::shared_mutex> lock(mMutex);
			_eraseUnlocked(handle);
		}

		void clear() {
			std::unique_lock<std::shared_mutex> lock(mMutex);
			mEntryPositions.clear();
			mEntries.clear();
			mFramesUntilEviction.clear();
		}

		// Ages every entry by one frame. Anything that reaches zero is handed to destroy()
		template<typename DestroyFunc>
		void age(DestroyFunc&& destroy) {
			static_assert(kTracksEviction, "Cache does not evict - nothing to age");
			std::unique_lock<std::shared_mutex> lock(mMutex);
			// Walk backwards: erase() swaps the last entry into the hole
			for (size_t i = mEntries.size(); i-- > 0; ) {
				if (mFramesUntilEviction[i] == 0) {
					Entry& entry = *mEntries[i];
					const Handle handle = entry.mHandle;
					destroy(entry);
					_eraseUnlocked(handle);
				}
				else {
					mFramesUntilEviction[i]--;
				}
			}
		}

	private:
		// Callers must already hold the lock exclusively
		void _eraseUnlocked(Handle handle) {
			auto entryPos = _getEntryPosition(handle);
			if (entryPos == kInvalidEntryPosition) {
				return;
			}

			// Move the erased entry to the tail and pop_back()
			const EntryPosition last = static_cast<EntryPosition>(mEntries.size() - 1);
			if (entryPos != last) {
				mEntries[entryPos] = std::move(mEntries[last]);
				if constexpr (kTracksEviction) {
					mFramesUntilEviction[entryPos] = mFramesUntilEviction[last];
				}
				mEntryPositions.insert_or_assign(mEntries[entryPos]->mHandle, entryPos);
			}
			mEntries.pop_back();
			if constexpr (kTracksEviction) {
				mFramesUntilEviction.pop_back();
			}
			mEntryPositions.erase(handle);
		}

		[[nodiscard]] EntryPosition _getEntryPosition(Handle handle) const {
			const auto it = mEntryPositions.find(handle);
			return it == mEntryPositions.cend() ? kInvalidEntryPosition : it->second;
		}

		void _markUsed(EntryPosition entryPos) const {
			if constexpr (kTracksEviction) {
				mFramesUntilEviction[entryPos] = RetainFrames;
			}
			else {
				static_cast<void>(entryPos);
			}
		}

		mutable std::shared_mutex mMutex;
		entt::dense_map<Handle, EntryPosition> mEntryPositions;
		// unique_ptr because resolve() hands back a pointer and render code holds references across
		// passes - a flat vector would dangle every time the cache grows.
		std::vector<std::unique_ptr<Entry>> mEntries;
		mutable std::vector<uint8_t> mFramesUntilEviction;
	};
}
