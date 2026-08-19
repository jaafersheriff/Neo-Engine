#pragma once

#include <type_traits>
#include <utility>

namespace neo {

	// Deliberate empty tag as the base component so entt's empty-type optimization stores nothing for them
	// Use START_COMPONENT and END_COMPONENT for expected Component machinery
	// opt into widgets by declaring
	//     void imGuiEditor();
	struct Component {};

	// Detection for the opt-in above.
	template<typename CompT, typename = void>
	struct HasImGuiEditor : std::false_type {};

	template<typename CompT>
	struct HasImGuiEditor<CompT, std::void_t<decltype(std::declval<CompT&>().imGuiEditor())>> : std::true_type {};

	template<typename CompT>
	inline constexpr bool HasImGuiEditor_v = HasImGuiEditor<CompT>::value;

#define START_COMPONENT(inComponent) \
	struct inComponent : public neo::Component { \
		static constexpr const char* kName = #inComponent

#define END_COMPONENT() }
}
