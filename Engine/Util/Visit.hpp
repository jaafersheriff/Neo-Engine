#pragma once

#include <utility>
#include <variant>

namespace neo {
	namespace util {

		// https://en.cppreference.com/w/cpp/utility/variant/visit
		template<class... Ts>
		struct VisitOverloaded : Ts... { using Ts::operator()...; };
		template<class... Ts>
		VisitOverloaded(Ts...) -> VisitOverloaded<Ts...>;

		template <class T, class... Ts>
		constexpr auto visit(T&& t, Ts&&... funcs) {
			return std::visit(VisitOverloaded{std::forward<Ts>(funcs)...}, t);
		}
	}
}
