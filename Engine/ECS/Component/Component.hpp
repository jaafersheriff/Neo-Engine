#pragma once

#include <type_traits>
#include <utility>

namespace neo {
	class ECS;

	// Deliberate empty tag as the base component so entt's empty-type optimization stores nothing for them
	// Use START_COMPONENT and END_COMPONENT for expected Component machinery
	struct Component {};

	// Detection for the opt-in to widgets
	// Declared via imGuiEditor()
	template<typename CompT, typename = void>
	struct HasImGuiEditor : std::false_type {};

	template<typename CompT>
	struct HasImGuiEditor<CompT, std::void_t<decltype(std::declval<CompT&>().imGuiEditor())>> : std::true_type {};

	template<typename CompT>
	inline constexpr bool HasImGuiEditor_v = HasImGuiEditor<CompT>::value;

	// Detection for opt-in to responding to Messages
	// Declared via static void registerMessageHandlers(ECS&);
	template<typename CompT, typename = void>
	struct HasMessageHandlers : std::false_type {};

	template<typename CompT>
	struct HasMessageHandlers<CompT, std::void_t<decltype(CompT::registerMessageHandlers(std::declval<ECS&>()))>> : std::true_type {};

	template<typename CompT>
	inline constexpr bool HasMessageHandlers_v = HasMessageHandlers<CompT>::value;

#define START_COMPONENT(inComponent) \
	struct inComponent : public neo::Component { \
		static constexpr const char* kName = #inComponent

#define END_COMPONENT() }
}
