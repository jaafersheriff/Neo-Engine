#pragma once

#include <entt/container/dense_map.hpp>

#include <chrono>
#include <cstdint>
#include <memory>
#include <optional>
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

		// Iteration hands back whole entries and deliberately does NOT count as a use to avoid influencing pooling
		template<typename EntryT>
		class Iterator {
		public:
			Iterator(const std::unique_ptr<Entry>* entries, size_t index)
				: mEntries(entries)
				, mIndex(index)
			{}

			EntryT& operator*() const { return *mEntries[mIndex]; }
			EntryT* operator->() const { return mEntries[mIndex].get(); }

			Iterator& operator++() {
				mIndex++;
				return *this;
			}

			bool operator==(const Iterator& other) const { return mIndex == other.mIndex; }
			bool operator!=(const Iterator& other) const { return mIndex != other.mIndex; }

		private:
			const std::unique_ptr<Entry>* mEntries;
			size_t mIndex;
		};

		using iterator = Iterator<Entry>;
		using const_iterator = Iterator<const Entry>;

		[[nodiscard]] iterator begin() { return iterator(mEntries.data(), 0); }
		[[nodiscard]] iterator end() { return iterator(mEntries.data(), mEntries.size()); }
		[[nodiscard]] const_iterator begin() const { return const_iterator(mEntries.data(), 0); }
		[[nodiscard]] const_iterator end() const { return const_iterator(mEntries.data(), mEntries.size()); }

		[[nodiscard]] bool contains(Handle handle) const {
			return mEntryPositions.contains(handle);
		}

		[[nodiscard]] size_t size() const {
			return mEntries.size();
		}

		// The only path that counts as "still in use".
		[[nodiscard]] const Entry* resolve(Handle handle) const {
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
			const EntryPosition entryPos = _getEntryPosition(handle);
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

		void clear() {
			mEntryPositions.clear();
			mEntries.clear();
			mFramesUntilEviction.clear();
		}

		// Ages every entry by one frame. Anything that reaches zero is handed to destroy() callback
		template<typename DestroyFunc>
		void age(DestroyFunc&& destroy) {
			static_assert(kTracksEviction, "Cache does not evict - nothing to age");
			// Walk backwards: erase() swaps the last entry into the hole
			for (size_t i = mEntries.size(); i-- > 0; ) {
				if (mFramesUntilEviction[i] == 0) {
					Entry& entry = *mEntries[i];
					const Handle handle = entry.mHandle;
					destroy(entry);
					erase(handle);
				}
				else {
					mFramesUntilEviction[i]--;
				}
			}
		}

	private:
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

		entt::dense_map<Handle, EntryPosition> mEntryPositions;
		// unique_ptr because resolve() hands back a pointer and render code holds references across
		// passes - a flat vector would dangle every time the cache grows.
		std::vector<std::unique_ptr<Entry>> mEntries;
		mutable std::vector<uint8_t> mFramesUntilEviction;
	};
}
