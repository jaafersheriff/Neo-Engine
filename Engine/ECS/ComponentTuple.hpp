#pragma once

#include "ECS/Component/Component.hpp"

#include <optional>
#include <unordered_map>

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
}
