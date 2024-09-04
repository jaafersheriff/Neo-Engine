#include <ext/entt_incl.hpp>
#include <entt/locator/locator.hpp>

namespace neo {
	template<typename T>
	using ServiceLocator = entt::locator<T>;
}
