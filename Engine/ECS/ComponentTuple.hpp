#pragma once

#include "ECS/Component/Component.hpp"

#include <optional>
#include <unordered_map>
#include <entt/entt.hpp>

namespace neo {

	template<typename... CompTs>
	struct ComponentTuple {
		entt::entity mEntity;
		std::tuple<CompTs*...> mTuple = {};
		mutable bool mValid = true;

		template<typename ...CompTs>
		ComponentTuple(entt::entity e, std::tuple<CompTs*...>& inTuple)
			: mEntity(e)
			, mTuple(inTuple)
		{
			_validate<CompTs...>();
		}

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
}
