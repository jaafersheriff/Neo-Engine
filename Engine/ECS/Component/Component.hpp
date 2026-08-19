#pragma once

#include <type_traits>
#include <utility>

namespace neo {

	// An empty tag base, deliberately: no vtable, no members. A component is data, and everything the
	// engine needs to *do* with one is dispatched statically through ECS::ComponentRegistry, which
	// always has the concrete type in hand. Keeping this empty is what lets the pure-tag components
	// actually be empty types, so EnTT's empty-type optimisation stores nothing at all for them.
	//
	// A component opts into an inspector widget by declaring - no virtual, no override:
	//     void imGuiEditor();
	// ComponentRegistry detects that signature and wires the widget up; a component without one just
	// has no widget. That opt-in is the entire API this base used to advertise as a virtual.
	struct Component {};

	// Detection for the opt-in above.
	template<typename CompT, typename = void>
	struct HasImGuiEditor : std::false_type {};

	template<typename CompT>
	struct HasImGuiEditor<CompT, std::void_t<decltype(std::declval<CompT&>().imGuiEditor())>> : std::true_type {};

	template<typename CompT>
	inline constexpr bool HasImGuiEditor_v = HasImGuiEditor<CompT>::value;

	// kName is static on purpose. The string is identical for every instance of a type and
	// ComponentRegistry already keeps one copy per type, so as a member it cost 8 bytes on every
	// component of every entity - and the clone copied all of them, every frame.
#define START_COMPONENT(inComponent) \
	struct inComponent : public neo::Component { \
		static constexpr const char* kName = #inComponent

#define END_COMPONENT() }
}
