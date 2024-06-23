#pragma once
namespace neo {

	struct Component {
		/* Components can have an editor */
		virtual void imGuiEditor() {};
	};


#define START_COMPONENT(inComponent) \
	struct inComponent : public Component { \
		const char* mName = #inComponent

#define END_COMPONENT() }
}
