#pragma once

#include "Messaging/Message.hpp"

#define NUM_KEYS /*GLFW_KEY_LAST*/ 348

namespace neo {

	class Keyboard {
	public:
		struct ResetKeyboardMessage : public Message { };
		struct KeyPressedMessage : public Message {
			int mKey;
			int mAction;
			KeyPressedMessage(int key, int action)
				: mKey(key)
				, mAction(action)
			{}
		};

		Keyboard() = default;
		~Keyboard() = default;
		Keyboard(const Keyboard&) = default;
		Keyboard& operator=(const Keyboard&) = default;
		
		void init();

		bool isKeyPressed(int) const;

	private:
		bool mKeyStatus[NUM_KEYS] = {/*GLFW_RELEASE*/ 0};

		void _onReset(const ResetKeyboardMessage& msg);
		void _onKeyPressed(const KeyPressedMessage& msg);
	};
}
