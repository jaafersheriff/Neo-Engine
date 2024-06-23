#pragma once

#include "ECS/Component/Component.hpp"
#include "Hardware/Keyboard.hpp"

namespace neo {

	START_COMPONENT(KeyboardComponent);
		KeyboardComponent(Keyboard engineKeyboard)
			: mFrameKeyboard(engineKeyboard)
		{}

		Keyboard mFrameKeyboard;
	END_COMPONENT();
}