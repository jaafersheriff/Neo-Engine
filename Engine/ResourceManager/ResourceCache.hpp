#pragma once

#include <ext/entt_incl.hpp>
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
	// callers reach a resource by handle or through forEach.
	//
	// Threading. The render thread mutates the cache from ResourceManagerInterface::tick, while the
	// main thread resolves out of it (ECS systems, demo update, the ImGui panels) and the shader
	// hot-reload thread walks it on its own schedule. A shared_mutex covers that: resolve, contains,
	// size and forEach take it shared, and insert, erase, clear and age take it exclusively.
	//
	// resolve() is the one reader that also writes - it refills the entry's eviction counter - so on
	// a cache that actually evicts it takes the lock exclusively instead. That is deliberately the
	// cheap case: only framebuffers and shader buffers evict, and they are resolved a handful of
	// times per pass. The per-draw path (textures, meshes, shaders) never evicts and stays shared.
	//
	// Iterators are NOT exposed. Handing one out would mean handing out access that outlives the
	// lock; forEach keeps the lock for the whole walk instead.
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


		// Walks every entry with the lock held for the whole traversal. Deliberately does NOT count as
		// a use, or a resource listed in an ImGui panel could never age out while the panel is open.
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
			return mSlotByHandle.contains(handle);
		}

		[[nodiscard]] size_t size() const {
			std::shared_lock<std::shared_mutex> lock(mMutex);
			return mEntries.size();
		}

		// The only path that counts as "still in use".
		[[nodiscard]] const Entry* resolve(Handle handle) const {
			// An evicting cache writes the counter here, so it needs the lock exclusively; everything
			// else is a pure read and stays shared. See the threading note above.
			// TODO - not great for perf
			std::conditional_t<kTracksEviction, std::unique_lock<std::shared_mutex>, std::shared_lock<std::shared_mutex>> lock(mMutex);
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
			std::unique_lock<std::shared_mutex> lock(mMutex);
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

		// Removes a resource from the lookup and hands ownership to the caller, under the exclusive
		// lock. The resource itself is untouched - still alive, just unreachable through resolve().
		// That split is the point: destroying it is deferred, but it stops being findable *now*, so
		// nobody can acquire a fresh pointer to something already doomed.
		[[nodiscard]] std::unique_ptr<Entry> extract(Handle handle) {
			std::unique_lock<std::shared_mutex> lock(mMutex);
			const Slot slot = _slot(handle);
			if (slot == kInvalidSlot) {
				return nullptr;
			}
			std::unique_ptr<Entry> extracted = std::move(mEntries[slot]);
			_eraseUnlocked(handle);
			return extracted;
		}

		void erase(Handle handle) {
			std::unique_lock<std::shared_mutex> lock(mMutex);
			_eraseUnlocked(handle);
		}

		void clear() {
			std::unique_lock<std::shared_mutex> lock(mMutex);
			mSlotByHandle.clear();
			mEntries.clear();
			mFramesUntilEviction.clear();
		}

		// Ages every entry by one frame and reports the handles that expired. It deliberately does
		// not destroy or erase anything - the caller retires them through extract(), so eviction and
		// explicit discard take exactly the same path.
		template<typename ExpiredFunc>
		void age(ExpiredFunc&& onExpired) const {
			static_assert(kTracksEviction, "Cache does not evict - nothing to age");
			std::shared_lock<std::shared_mutex> lock(mMutex);
			for (size_t i = 0; i < mEntries.size(); i++) {
				if (mFramesUntilEviction[i] == 0) {
					onExpired(mEntries[i]->mHandle);
				}
				else {
					mFramesUntilEviction[i]--;
				}
			}
		}

	private:
		// Callers must already hold the lock exclusively. age() erases as it sweeps, so it cannot go
		// back through the public erase().
		void _eraseUnlocked(Handle handle) {
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

		mutable std::shared_mutex mMutex;
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
