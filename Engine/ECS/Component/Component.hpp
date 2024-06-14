#pragma once
#include <string>

namespace neo {

	struct Component {
		/* Components can have an editor */
		virtual std::string getName() const = 0;
		virtual void imGuiEditor() {};
	};
}
