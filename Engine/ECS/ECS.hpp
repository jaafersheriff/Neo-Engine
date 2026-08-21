#pragma once

#include "ECS/Systems/System.hpp"

#include "Jobs/JobSystem.hpp"

#include "Util/Profiler.hpp"
#include "Util/ServiceLocator.hpp"
#include "Util/Util.hpp"

#include <ext/entt_incl.hpp>
#include <entt/entt.hpp>

#include <ext/imgui_incl.hpp>

#include <algorithm>
#include <cstring>
#include <typeindex>
#include <optional>
#include <mutex>

namespace neo {
	class System;
	class Engine;

	class ECS {
		friend Engine;

	public:
		using Entity = entt::entity;
		using Registry = entt::registry;
		using ComponentTypeID = HashedString::hash_type;

		static constexpr Entity NEO_INVALID_ENTITY = entt::null;

		class EntityBuilder {
			friend ECS;
		public:
			EntityBuilder() = default;
			EntityBuilder(const EntityBuilder& other) {
				this->mComponents = other.mComponents;
			}

			template<typename CompT, typename... Args>
			EntityBuilder& attachComponent(Args &&... args) {
				// Holy moly wtf c++
				mComponents.emplace_back([args = std::make_tuple(std::forward<Args>(args) ...)](ECS& ecs, ECS::Entity e) mutable {
					std::apply([&ecs, e](auto&& ... args) {
						ecs.addComponent<CompT>(e, std::forward<decltype(args)>(args) ...);
						}, std::move(args));
					});

				return *this;
			}
	
		private:
			using AttachFunc = std::function<void(ECS& ecs, ECS::Entity e)>;
			std::vector<AttachFunc> mComponents;
		};


		ECS() = default;
		~ECS() = default;
		ECS(const ECS&) = delete;
		ECS& operator=(const ECS&) = delete;

		void submitEntity(EntityBuilder&& builder);
		void removeEntity(Entity e);

		// Entity access
		template<typename CompT, typename... Args> CompT* addComponent(Entity e, Args &&... args);
		template<typename CompT> void removeComponent(Entity e);

		template<typename CompT> bool has(Entity e) const;
		template<typename CompT> CompT* getComponent(Entity e);
		template<typename CompT> CompT *const cGetComponent(Entity e) const;

		// All access
		template<typename CompT> std::optional<std::tuple<Entity, CompT&>> getComponent();
		template<typename CompT> std::optional<std::tuple<ECS::Entity, const CompT&>> cGetComponent() const;
		template<typename CompT> uint32_t entityCount() const;
		template<typename CompT> bool has() const;
		template<typename CompTA, typename CompTB, typename... CompTs> bool has() const;
		template<typename... CompTs> auto getView();
		template<typename... CompTs> const auto getView() const;
		template<typename... CompTs> std::optional<std::tuple<Entity, CompTs&...>> getSingleView();
		template<typename... CompTs> std::optional<std::tuple<Entity, const CompTs&...>> getSingleView() const;

		template<typename FilterCompT, typename SortCompT> void sort(std::function<bool(Entity left, Entity right)> compare) const;

		/* Iterate a view across job system workers, or serially when there are too few entities for that
		   to pay. The decision is made per call from the entity count, so a system that usually sees a
		   dozen entities and occasionally sees fifty thousand needs no special casing.

		   This is a template all the way down on purpose: the type erasure happens once per *batch*, at
		   the job boundary, so the per-entity body still inlines.

		   The contract, none of which is checked for you:
		   - Structural changes are safe, but they are not free. addComponent, removeComponent,
			 submitEntity and removeEntity all stage into mutex-guarded queues, and the per-type
			 registry behind addComponent is guarded too, so calling one from a worker is correct -
			 every worker just serialises on the same lock while it does. Keep them on paths that are
			 rarely taken rather than on every entity. One caveat that is not about locking: the first
			 sighting of a component type is what runs its registerMessageHandlers, so a type that has
			 handlers wants to be seen on the main thread before it is ever added from a wide pass.
		   - No reading another entity's components. Each iteration touches only its own.
		   - Accumulate into buckets indexed by JobSystem::threadIndex(), never a shared counter, and
			 combine afterwards. A reduction must also combine deterministically - order of arrival is
			 not stable between frames.
		   - Nothing may depend on visit order. The wide path walks the leading pool's packed array
			 forwards, which is the reverse of what iterating the view gives you, and the batches of it
			 land in whatever order the workers get to them. */
		template<typename... CompTs, typename Fn> void parallelForEach(Fn&& fn, uint32_t batchSize = sParallelBatchSize);

		/* Both live in the ECS ImGui pane so they get chosen by measurement. Setting the threshold to 0
		   forces every parallelForEach wide, which is how the two paths get compared. */
		static uint32_t sParallelGoWideThreshold;
		static uint32_t sParallelBatchSize;

		/* Attach a system */
		template <typename SysT, typename... Args> SysT& addSystem(Args &&...);
		template <typename SysT> bool isSystemEnabled() const;
		template <typename SysT> void setSystemActive(bool active);


	private:
		mutable Registry mRegistry;

		/* Active containers */
		std::mutex mEntityCreationMutex;
		std::vector<EntityBuilder> mEntityCreateQueue;

		std::mutex mEntityKillMutex;
		std::vector<Entity> mEntityKillQueue;

		using ComponentModFunc = std::function<void(Registry&)>;
		std::mutex mAddComponentMutex;
		std::vector<ComponentModFunc> mAddComponentFuncs;

		std::mutex mRemoveComponentMutex;
		std::vector<ComponentModFunc> mRemoveComponentFuncs;

		std::vector<std::pair<std::type_index, std::unique_ptr<System>>> mSystems;
		void _initSystems();
		void _updateSystems(const ResourceManagers& resourceManagers);

		// Deep-copies this ECS's registry into dst, preserving entity identifiers exactly
		void _cloneInto(ECS& dst) const;


		void _flush();
		void _clean();

		void _imguiEdtor();
		void _imguiComponentEditor(Entity e);

		// ImGui tooling
		class ComponentRegistry {
			friend ECS;
		public:
			struct Entry {
				const char* mName = nullptr;
				void (*mWidget)(ECS&, Entity) = nullptr;
				void (*mRemove)(ECS&, Entity) = nullptr;
				void (*mClone)(const Registry& src, Registry& dst) = nullptr;
			};

			[[nodiscard]] auto begin() const { return mEntries.cbegin(); }
			[[nodiscard]] auto end() const { return mEntries.cend(); }

		private:
			// One dense_map probe in the common case where the type has been seen before. Returns
			// true only on the first sighting, which is the hook for one-time per-type setup.
			//
			// Locked because addComponent is reachable from a job worker, and this is the half of it
			// that the queue's mutex does not cover: an insert here can rehash, so two workers meeting
			// an unseen type at the same moment would corrupt the map. Only the mutating path takes the
			// lock - begin()/end() do not, because the clone and the inspector walk this on the main
			// thread at points where no wide pass is in flight.
			bool _ensure(ComponentTypeID type, const Entry& entry) {
				std::lock_guard<std::mutex> lock(mEnsureMutex);
				if (mEntries.contains(type)) {
					return false;
				}
				mEntries.insert_or_assign(type, entry);
				return true;
			}

			void _clear() {
				mEntries.clear();
			}

			entt::dense_map<ComponentTypeID, Entry> mEntries;
			std::mutex mEnsureMutex;
		};
		ComponentRegistry mComponentRegistry;
		// Type-erased entry points for ComponentRegistry
		template<typename CompT> static void _componentWidget(ECS& ecs, Entity e);
		template<typename CompT> static void _componentRemove(ECS& ecs, Entity e);
		template<typename CompT> static void _componentClone(const Registry& src, Registry& dst);

	};

	template<typename CompT>
	std::optional<std::tuple<ECS::Entity, CompT&>> ECS::getComponent() {
		static_assert(std::is_base_of<Component, CompT>::value, "CompT must be a component type");
		static_assert(!std::is_same<CompT, Component>::value, "CompT must be a derived component type");
	
		auto view = mRegistry.view<CompT>();
		if (view.size() > 1) {
			NEO_LOG_E("Attempting to get a single %s when multiple exist", CompT::kName);
		}
		if (view.size()) {
			return { *view.each().begin() };
		}
		return std::nullopt;
	}

	template<typename CompT>
	std::optional<std::tuple<ECS::Entity, const CompT&>> ECS::cGetComponent() const{
		static_assert(std::is_base_of<Component, CompT>::value, "CompT must be a component type");
		static_assert(!std::is_same<CompT, Component>::value, "CompT must be a derived component type");
	
		auto view = mRegistry.view<CompT>();
		if (view.size() > 1) {
			NEO_LOG_E("Attempting to get a single %s when multiple exist", CompT::kName);
		}
		if (view.size()) {
			return { *view.each().begin() };
		}
		return std::nullopt;
	}

	template<typename CompT>
	bool ECS::has(ECS::Entity e) const {
		static_assert(std::is_base_of<Component, CompT>::value, "CompT must be a component type");
		static_assert(!std::is_same<CompT, Component>::value, "CompT must be a derived component type");

		return mRegistry.try_get<CompT>(e) != nullptr;
	}

	template <typename SysT, typename... Args> 
	SysT& ECS::addSystem(Args &&... args) {
		static_assert(std::is_base_of<System, SysT>::value, "SysT must be a System type");
		static_assert(!std::is_same<SysT, System>::value, "SysT must be a derived System type");
		std::type_index typeI(typeid(SysT));
		for (auto & sys : mSystems) {
			if (sys.first == typeI) {
				NEO_LOG_E("Attempting to add a duplicate system %s", sys.second->mName.c_str());
				return static_cast<SysT &>(*sys.second);
			}
		}

		mSystems.push_back({ typeI, std::make_unique<SysT>(std::forward<Args>(args)...) });
		return static_cast<SysT &>(*mSystems.back().second);
	}

	template <typename SysT> 
	bool ECS::isSystemEnabled() const {
		static_assert(std::is_base_of<System, SysT>::value, "SysT must be a System type");
		static_assert(!std::is_same<SysT, System>::value, "SysT must be a derived System type");
		std::type_index typeI(typeid(SysT));
		for (auto & sys : mSystems) {
			if (sys.first == typeI && sys.second->mActive) {
				return true;
			}
		}
		return false;
	}

	template <typename SysT> 
	void ECS::setSystemActive(bool active) {
		static_assert(std::is_base_of<System, SysT>::value, "SysT must be a System type");
		static_assert(!std::is_same<SysT, System>::value, "SysT must be a derived System type");
		std::type_index typeI(typeid(SysT));
		for (auto & sys : mSystems) {
			if (sys.first == typeI) {
				sys.second->mActive = active;
			}
		}
	}

	template<typename CompT>
	CompT* ECS::getComponent(Entity e) {
		static_assert(std::is_base_of<Component, CompT>::value, "CompT must be a component type");
		static_assert(!std::is_same<CompT, Component>::value, "CompT must be a derived component type");

		return mRegistry.try_get<CompT>(e);
	}

	template<typename CompT>
	CompT *const ECS::cGetComponent(Entity e) const {
		static_assert(std::is_base_of<Component, CompT>::value, "CompT must be a component type");
		static_assert(!std::is_same<CompT, Component>::value, "CompT must be a derived component type");

		return const_cast<CompT *const>(mRegistry.try_get<CompT>(e));
	}

	template<typename CompT, typename... Args>
	CompT* ECS::addComponent(Entity e, Args &&... args) {
		static_assert(std::is_base_of<Component, CompT>::value, "CompT must be a component type");
		static_assert(!std::is_same<CompT, Component>::value, "CompT must be a derived component type");

		CompT* component;
		if constexpr (sizeof...(Args) > 0) {
			component = new CompT(std::forward<Args>(args)...);
		}
		else {
			component = new CompT();
		}

		if (mComponentRegistry._ensure(
				entt::type_hash<CompT>::value(),
				ComponentRegistry::Entry{ CompT::kName, &ECS::_componentWidget<CompT>, &ECS::_componentRemove<CompT>, &ECS::_componentClone<CompT> })) {
			// First time anything has attached this type. If it needs render-discovered state written
			// back, this is where that gets wired up - the demo never has to know.
			if constexpr (HasMessageHandlers_v<CompT>) {
				CompT::registerMessageHandlers(*this);
			}
		}

		{
			std::lock_guard<std::mutex> lock(mAddComponentMutex);
			mAddComponentFuncs.emplace_back([e, component](Registry& registry) mutable {
				if (registry.try_get<CompT>(e)) {
					NEO_LOG_E("Attempting to add a second %s to entity %d when one already exists", CompT::kName, e);
				}
				else {
					registry.emplace<CompT>(e, *component);
				}

				delete component;
				component = nullptr;
			});
		}

		return component;
	}

	template<typename CompT>
	void ECS::removeComponent(Entity e) {
		static_assert(std::is_base_of<Component, CompT>::value, "CompT must be a component type");
		static_assert(!std::is_same<CompT, Component>::value, "CompT must be a derived component type");
		std::lock_guard<std::mutex> lock(mRemoveComponentMutex);
		mRemoveComponentFuncs.push_back([e](Registry& registry) mutable {
			registry.remove<CompT>(e);
		});
	}

	template<typename... CompTs>
	auto ECS::getView() {
		TRACY_ZONE();
		return mRegistry.view<CompTs...>();
	}

	template<typename... CompTs>
	const auto ECS::getView() const {
		TRACY_ZONE();
		// TODO -- maybe force const inputs rather than attaching it
		// TODO -- otherwise view.get<> breaks
		return mRegistry.view<CompTs...>();
	}

	template<typename... CompTs, typename Fn>
	void ECS::parallelForEach(Fn&& fn, uint32_t batchSize) {
		TRACY_ZONE();

		auto view = getView<CompTs...>();

		// A view is not randomly accessible, but the storage behind it is. A view over N component types
		// is one leading pool - the smallest of them - plus a per-element test that the other N-1 pools
		// hold the same entity, and a pool is a packed array. So the batches index that array and repeat
		// the test themselves, instead of collecting the view into a vector the batches can index. The
		// collect was a second full pass over every entity and measured as one, at roughly a third of
		// the whole call on DrawStress.
		const auto* pool = view.handle();

		// Exactly what the view would iterate, not an estimate. A swap_only pool keeps its dead entities
		// in the packed array past the free list, so only the live prefix counts; every other policy
		// iterates the whole array and filters below. Derived here rather than taken from size_hint()
		// because a single-component view only declares size_hint() for in-place pools, and this is the
		// one formulation that compiles for any CompTs.
		const uint32_t count = pool == nullptr
			? 0u
			: static_cast<uint32_t>(pool->policy() == entt::deletion_policy::swap_only ? pool->free_list() : pool->size());

		if (count < sParallelGoWideThreshold) {
			// Small views never touch the scheduler at all - waking workers to move forty entities costs
			// more than moving them. Straight off the view, so the serial path stays the plain thing the
			// wide path is checked against.
			for (const Entity entity : view) {
				fn(entity, view.template get<CompTs>(entity)...);
			}
			return;
		}

		// pool is non-null here: the only way it is null is an empty view, which count == 0 has already
		// sent down the serial path - and parallelFor returns immediately on a count of 0 regardless, so
		// forcing the threshold to 0 in the ImGui pane cannot reach a dereference either.
		ServiceLocator<JobSystem>::ref().parallelFor(count, batchSize, [pool, &view, &fn](uint32_t begin, uint32_t end, uint32_t) {
			for (uint32_t i = begin; i < end; ++i) {
				const Entity entity = (*pool)[i];
				// The test the view iterator would have applied, and it rejects three things at once:
				// tombstones left behind by in-place deletion, entities the leading pool holds but the
				// others do not, and slots a swap_only pool has already freed. Dropping it would hand fn
				// a dead entity and make the get below undefined rather than loud.
				if (!view.contains(entity)) {
					continue;
				}
				fn(entity, view.template get<CompTs>(entity)...);
			}
		});
	}

	template<typename... CompTs>
	std::optional<std::tuple<ECS::Entity, CompTs&...>> ECS::getSingleView() {
		TRACY_ZONE();
		auto view = mRegistry.view<CompTs...>();
		if (view.size_hint() > 1) {
			NEO_LOG_E("Found %d entities when one was requested in %s", view.size_hint(), __FUNCSIG__);
		}
		if (view.begin() != view.end()) {
			return { *view.each().begin() };
		}
		return std::nullopt;
	}

	// TODO -- maybe force const inputs rather than attaching it
	// TODO -- otherwise view.get<> breaks
	template<typename... CompTs>
	std::optional<std::tuple<ECS::Entity, const CompTs&...>> ECS::getSingleView() const {
		TRACY_ZONE();
		auto view = mRegistry.view<const CompTs...>();
		if (view.size_hint() > 1) {
			NEO_LOG_E("Found %d entities when one was requested in %s", view.size_hint(), __FUNCSIG__);
		}
		if (view.begin() != view.end()) {
			return { *view.each().begin() };
		}
		return std::nullopt;
	}

	template<typename CompT> 
	uint32_t ECS::entityCount() const {
		return static_cast<uint32_t>(mRegistry.storage<CompT>().size());
	}

	template<typename CompT>
	bool ECS::has() const {
		return entityCount<CompT>() > 0;
	}

	template<typename CompTA, typename CompTB, typename... CompTs> 
	bool ECS::has() const {
		return has<CompTA>() && has<ComptB> && has<CompTs...>();
	}

	template<typename FilterCompT, typename SortCompT> 
	void ECS::sort(std::function<bool(const Entity left, const Entity right)> compare) const {
		mRegistry.sort<SortCompT>(compare);
		// EnTT can only sort against a single component ;( and then FilterCompT will be sorted against SortCompT
		mRegistry.sort<FilterCompT, SortCompT>();
	}

	template<typename CompT>
	void ECS::_componentWidget(ECS& ecs, Entity e) {
		if constexpr (HasImGuiEditor_v<CompT>) {
			if (CompT* component = ecs.getComponent<CompT>(e)) {
				component->imGuiEditor();
			}
		}
		else {
			NEO_UNUSED(ecs, e);
		}
	}

	template<typename CompT>
	void ECS::_componentRemove(ECS& ecs, Entity e) {
		ecs.removeComponent<CompT>(e);
	}

	template<typename CompT>
	void ECS::_componentClone(const Registry& src, Registry& dst) {
		// Assures the pool exists in dst. Note it is deliberately not cleared yet - the fast path below
		// needs to see what the destination is still holding from two frames ago.
		auto& to = dst.storage<CompT>();

		const auto* from = src.storage<CompT>();
		if (from == nullptr || from->empty()) {
			to.clear();
			return;
		}

		// The expensive half of a rebuild is not the elements, it is the sparse array: clear() walks the
		// packed array writing null into a sparse slot per entity, and insert() then walks it again
		// writing the position back. Two passes of scattered writes across sparse pages, per pool, per
		// frame - and both of them reconstruct a mapping that is usually bit-for-bit what was already
		// there, because entities come and go far more slowly than their components change value.
		//
		// So compare first. If the destination's packed entity array already matches the source's, the
		// sparse array matches too - a sparse slot holds nothing but the entity's packed position, and
		// every pool here only ever contains what a previous clone put in it - and the whole structure
		// can be left alone. What is left is a positional overwrite of the elements, which is sequential
		// and the part that actually had to happen.
		//
		// EnTT offers nothing cheaper than this. A storage is move-only (copy construction and copy
		// assignment are both deleted) and both the element payload and the sparse array are private
		// paged containers, so there is no route from out here to a page-level memcpy of a pool.
		if constexpr (std::is_copy_assignable_v<CompT>) {
			// swap_and_pop is what every component pool gets, since the policy follows the type's
			// in_place_delete trait and nothing in Neo sets it. Checked rather than assumed because the
			// other two policies both break the comparison: in-place deletion leaves tombstones in the
			// packed array whose elements are destroyed, and swap_only keeps dead entities past the
			// free list. Either one would have this copy read from a dead slot.
			if (to.size() == from->size()
				&& from->policy() == entt::deletion_policy::swap_and_pop
				&& to.policy() == entt::deletion_policy::swap_and_pop
				&& std::memcmp(to.data(), from->data(), from->size() * sizeof(Entity)) == 0) {
				TRACY_ZONEN("Clone pool (in place)");
				if constexpr (entt::component_traits<CompT, Entity>::page_size != 0u) {
					// Both iterators run the same direction over the same index space, so this pairs
					// positionally with the entity array the memcmp just proved identical.
					std::copy(from->begin(), from->end(), to.begin());
				}
				// An empty component has no elements at all, so a matching entity array is the whole job.
				return;
			}
		}

		TRACY_ZONEN("Clone pool (rebuild)");
		// Cleared rather than destroyed - the packed capacity survives, so a steady-state frame reuses
		// the same memory every time.
		to.clear();
		to.reserve(from->size());

		// Two ranges over one packed index space: the sliced base walks the entity array, the storage
		// walks the element array. Both iterators are the same shape (operator++ decrements an offset,
		// index() is offset - 1), so rbegin() pairs them positionally AND reproduces the source's
		// packed order rather than reversing it.
		const Registry::common_type& entities = *from;
		if constexpr (entt::component_traits<CompT, Entity>::page_size == 0u) {
			// Empty component: EnTT stores no elements for it, so the entity list is the whole payload.
			to.insert(entities.rbegin(), entities.rend());
		}
		else {
			to.insert(entities.rbegin(), entities.rend(), from->rbegin());
		}
	}
}
