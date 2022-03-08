#pragma once

#include <entt/entt.hpp>
#include <entt/entity/group.hpp>
#include <entt/entity/utility.hpp>

namespace neo {

	template<typename... CompTs>
	struct ComponentTuple {
		std::tuple<CompTs*...> mTuple = {};
		mutable bool mValid = true;

		template<typename ...CompTs>
		ComponentTuple(std::tuple<CompTs*...>& inTuple)
			: mTuple(inTuple)
		{
			_validate<CompTs...>();
		}
		ComponentTuple(const ComponentTuple& other) = delete;
		ComponentTuple(ComponentTuple&& other) = delete;

		~ComponentTuple() = default;

		operator bool() {
			return mValid;
		}


		template<typename CompT>
		CompT& get() {
			return *std::get<CompT*>(mTuple);
		}

		template<typename CompT>
		const CompT& get() const {
			return *std::get<CompT*>(mTuple);
		}


		auto raw() {
			return mTuple;
		}

	private:
		template<typename T, typename ...S>
		auto _validate() const {
			if (std::get<T*>(mTuple) == nullptr) {
				mValid = false;
			}
			else {
				if constexpr (sizeof...(S) > 0) {
					_validate<S...>();
				}
			}

		}
	};


	class ECS {
	public:
		using Entity = entt::entity;
		using Registry = entt::registry;

		template<typename... CompTs>
		using View = entt::basic_view<Entity, entt::get_t<CompTs...>, entt::exclude_t<>>;
		ECS() = default;
		ECS(const ECS& other) = delete;
		ECS(ECS&& other) = delete;
		~ECS() = default;

		void flush();

		Entity createEntity();
		void removeEntity(Entity e);

		// Entity access
		template<typename CompT, typename... Args> CompT& addComponent(Entity e, Args... args);
		template<typename CompT> void removeComponent(Entity e);
		template<typename CompT> CompT& getComponent(Entity e);
		template<typename CompT> const CompT& getComponent(Entity e) const;
		template<typename... CompTs> ComponentTuple<CompTs...> getComponentTuple(const Entity e);
		template<typename... CompTs> const ComponentTuple<CompTs...>& cGetComponentTuple(const Entity e) const;

		// All access
		template<typename... CompTs> View<CompTs...> getView();
		template<typename... CompTs> const View<CompTs...> getView() const;
		template<typename... CompTs> auto getSingleView();
		template<typename... CompTs> const auto getSingleView() const;
		template<typename... CompTs> std::vector<ComponentTuple<CompTs...>> getComponentTuples();
		template<typename... CompTs> std::vector<const ComponentTuple<CompTs...>> getComponentTuples() const;

		mutable Registry mRegistry;
	private:

		std::vector<Entity> mEntityKillQueue;

		using ComponentModFunc = std::function<void(Registry&)>;
		std::vector<ComponentModFunc> mAddComponentFuncs;
		std::vector<ComponentModFunc> mRemoveComponentFuncs;
	};

	struct Component {
	public:
		virtual std::string getName() = 0;
		virtual void imgui(ECS& ecs) = 0;
	};

	template<typename... CompTs>
	ComponentTuple<CompTs...> ECS::getComponentTuple(const Entity e) {
		return ComponentTuple<CompTs...>(std::move(mRegistry.try_get<CompTs...>(e)));
	}

	template<typename... CompTs>
	const ComponentTuple<CompTs...>& ECS::cGetComponentTuple(const Entity e) const {
		return ComponentTuple<CompTs...>(std::move(const_cast<Registry&>(mRegistry).try_get<CompTs...>(e)));
	}

	template<typename CompT>
	CompT& ECS::getComponent(Entity e) {
		if (auto comp = mRegistry.try_get<CompT>(e)) {
			return *comp;
		}
		throw std::runtime_error("Use getTuple dummy");
	}

	template<typename CompT>
	const CompT& ECS::getComponent(Entity e) const {
		if (auto comp = mRegistry.try_get<CompT>(e)) {
			return *comp;
		}
		throw std::runtime_error("Use getTuple dummy");
	}

	template<typename CompT, typename... Args>
	CompT& ECS::addComponent(Entity e, Args... args) {
		CompT* component;
		if constexpr (sizeof...(Args) > 0) {
			component = new CompT(std::forward<Args...>(args...));
		}
		else {
			component = new CompT();
		}

		mAddComponentFuncs.push_back([e, component](Registry& registry) mutable {
			if (registry.try_get<CompT>(e)) {
				throw std::runtime_error("Attempting to overwrite a component");
			}

			registry.emplace<CompT>(e, *component);
			delete component;
			component = nullptr;
			});

		return *component;
	}

	template<typename CompT>
	void ECS::removeComponent(Entity e) {
		mRemoveComponentFuncs.push_back([e](Registry& registry) mutable {
			registry.remove<CompT>(e);
			});
	}

	template<typename... CompTs>
	ECS::View<CompTs...> ECS::getView() {
		return mRegistry.view<CompTs...>();
	}

	template<typename... CompTs>
	const ECS::View<CompTs...> ECS::getView() const {
		return mRegistry.view<CompTs...>();
	}

	template<typename... CompTs>
	auto ECS::getSingleView() {
		return *mRegistry.view<CompTs...>().begin();
	}

	template<typename... CompTs>
	const auto ECS::getSingleView() const {
		return *mRegistry.view<CompTs...>().begin();
	}

	template<typename... CompTs>
	std::vector<ComponentTuple<CompTs...>> ECS::getComponentTuples() {
		std::vector<ECS::ComponentTuple<CompTs...>> ret;
		mRegistry.each([this, &ret](auto entity) {
			ret.push_back(getComponentTuple<CompTs...>(entity));
			});
		return ret;
	}

	template<typename... CompTs>
	std::vector<const ComponentTuple<CompTs...>> ECS::getComponentTuples() const {
		std::vector<ECS::ComponentTuple<CompTs...>> ret;
		mRegistry.each([this, &ret](auto entity) {
			ret.push_back(cGetComponentTuple<CompTs...>(entity));
			});
		return ret;
	}


}
