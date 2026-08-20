#pragma once

#include <ext/entt_incl.hpp>
#include <entt/locator/locator.hpp>

#include <utility>

namespace neo {

	template<typename T>
	class ServiceLocator {
	public:
		ServiceLocator() = delete;

		template<typename... Args>
		static void set(Args &&... args) {
			entt::locator<T>::emplace(std::forward<Args>(args)...);
		}

		[[nodiscard]] static T& ref() {
			return entt::locator<T>::value();
		}

		[[nodiscard]] static bool empty() {
			return !entt::locator<T>::has_value();
		}

		static void reset() {
			entt::locator<T>::reset();
		}
	};
}
