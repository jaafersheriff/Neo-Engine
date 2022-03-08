#pragma once

#include <string.h>

namespace neo {

	class Component {

		public:
			/* Components can have an editor */
			virtual void imGuiEditor() {};
			virtual std::string getName() = 0;
	};
}
