#pragma once

#include <string>

namespace neo {

	struct Component {
		/* Components can have an editor */
		virtual void imGuiEditor() {};
		virtual std::string getName() = 0;
	};
}
