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

	// Owned by ResourceManagerInterface.hpp, which is the public face of the resource system. Only
	// declared here: nothing in this header is instantiated until a manager declares its cache, by
	// which point that definition is in scope. This is what keeps the caller-facing handle type and
	// the cache-internal storage type in their own headers without a circular include.
	template<typename ResourceType>
	struct ResourceHandle;

	// `inline` rather than an anonymous namespace: CachedResource's constructor is an implicitly
	// inline template, so calling an internal-linkage function from it would give each translation
	// unit a different definition of that constructor.
	inline uint64_t getCurrentTimestamp() {
		using namespace std::chrono;
		return duration_cast<microseconds>(steady_clock::now().time_since_epoch()).count();
	}

	// How a resource is stored, which is deliberately not something the Renderer or the demos ever
	// see - they hold a ResourceHandle and ask a manager. One flat entry: the resource itself plus
	// everything the engine knows about it. There is exactly one "resource" here - `mResource` is
	// always the GL-backed object, never a wrapper.
	template<typename ResourceType>
	struct CachedResource {
		template<typename... Args>
		CachedResource(Args... args)
			: mResource(std::forward<Args>(args)...)
			, mCreationTimeStamp(getCurrentTimestamp())
		{}

		// Assigned by the cache on insert, so an entry always knows its own handle. That is what
		// lets iteration hand back a single object instead of a handle/resource pair.
		ResourceHandle<ResourceType> mHandle;
		ResourceType mResource;
		std::optional<std::string> mDebugName;
		const uint64_t mCreationTimeStamp;
	};

	// Handle-keyed storage for one resource type, with an optional eviction policy.
	//
	// RetainFrames == 0 means nothing is ever evicted: the only way out of the cache is an explicit
	// erase() driven by the manager's discard queue, and none of the eviction bookkeeping runs.
	//
	// RetainFrames > 0 means retain-after-last-use. Every resolve() refills that entry's counter to
	// RetainFrames, age() decrements every counter once per frame, and an entry whose counter reaches
	// zero is destroyed. The counter is one byte in its own packed array rather than a field inside
	// the entry, so the per-frame age() sweep touches 64 entries per cache line instead of striding
	// over whole resources to read one field.
	//
	// How entries are laid out inside is nobody else's business: the dense positions are private, and
	// callers reach a resource by handle or by iterating.
	template<typename ResourceType, uint8_t RetainFrames = 0>
	class ResourceCache {
		// Dense position of an entry. Unstable on purpose - erase() swaps the last entry into the
		// hole - which is exactly why it never leaves this class.
		using Slot = uint32_t;
		static constexpr Slot kInvalidSlot = UINT32_MAX;

	public:
		using Handle = ResourceHandle<ResourceType>;
		using Entry = CachedResource<ResourceType>;
		static constexpr bool kTracksEviction = RetainFrames > 0;

		// Iteration hands back whole entries - each one already knows its own handle - and
		// deliberately does NOT count as a use, or a resource listed in an ImGui panel could never
		// age out while the panel is open. Any insert(), erase() or age() invalidates iterators.
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
			return mSlotByHandle.contains(handle);
		}

		[[nodiscard]] size_t size() const {
			return mEntries.size();
		}

		// The only path that counts as "still in use".
		[[nodiscard]] const Entry* resolve(Handle handle) const {
			const Slot slot = _slot(handle);
			if (slot == kInvalidSlot) {
				return nullptr;
			}
			_markUsed(slot);
			return mEntries[slot].get();
		}

		[[nodiscard]] Entry* resolve(Handle handle) {
			return const_cast<Entry*>(std::as_const(*this).resolve(handle));
		}

		// Takes the finished entry by value and stamps it with its handle. A load that failed never
		// gets this far - the loader reports that upward to the manager, which logs it and skips.
		void insert(Handle handle, Entry&& entry) {
			entry.mHandle = handle;

			if (const auto it = mSlotByHandle.find(handle); it != mSlotByHandle.cend()) {
				mEntries[it->second] = std::make_unique<Entry>(std::move(entry));
				_markUsed(it->second);
				return;
			}

			mSlotByHandle.insert_or_assign(handle, static_cast<Slot>(mEntries.size()));
			mEntries.emplace_back(std::make_unique<Entry>(std::move(entry)));
			if constexpr (kTracksEviction) {
				mFramesUntilEviction.emplace_back(RetainFrames);
			}
		}

		void erase(Handle handle) {
			const auto it = mSlotByHandle.find(handle);
			if (it == mSlotByHandle.cend()) {
				return;
			}

			const Slot slot = it->second;
			const Slot last = static_cast<Slot>(mEntries.size() - 1);
			if (slot != last) {
				mEntries[slot] = std::move(mEntries[last]);
				if constexpr (kTracksEviction) {
					mFramesUntilEviction[slot] = mFramesUntilEviction[last];
				}
				mSlotByHandle.insert_or_assign(mEntries[slot]->mHandle, slot);
			}
			mEntries.pop_back();
			if constexpr (kTracksEviction) {
				mFramesUntilEviction.pop_back();
			}
			mSlotByHandle.erase(handle);
		}

		void clear() {
			mSlotByHandle.clear();
			mEntries.clear();
			mFramesUntilEviction.clear();
		}

		// Ages every entry by one frame. Anything that reaches zero is handed to destroy() - the GL
		// work belongs to the manager, not here - and then erased, so the caller never has to know
		// how entries are stored. Invalidates iterators.
		template<typename DestroyFunc>
		void age(DestroyFunc&& destroy) {
			static_assert(kTracksEviction, "Cache does not evict - nothing to age");
			// Walk backwards: erase() swaps the last entry into the hole, and everything above the
			// current position has already been visited, so nothing is skipped or aged twice.
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
		[[nodiscard]] Slot _slot(Handle handle) const {
			const auto it = mSlotByHandle.find(handle);
			return it == mSlotByHandle.cend() ? kInvalidSlot : it->second;
		}

		void _markUsed(Slot slot) const {
			if constexpr (kTracksEviction) {
				mFramesUntilEviction[slot] = RetainFrames;
			}
			else {
				static_cast<void>(slot);
			}
		}

		entt::dense_map<Handle, Slot> mSlotByHandle;
		// unique_ptr because resolve() hands back a pointer and render code holds references across
		// passes - a flat vector would dangle every time the cache grows.
		std::vector<std::unique_ptr<Entry>> mEntries;
		// Left empty and untouched when RetainFrames == 0. Usage tracking is bookkeeping about reads,
		// so a const resolve() legitimately writes it. Stage 5 turns these into relaxed atomics once
		// the render thread does the resolving.
		mutable std::vector<uint8_t> mFramesUntilEviction;
	};
}
