#pragma once

#ifdef EM

#include <entt/signal/dispatcher.hpp>

namespace neo {

	class EMessenger {
	public:
            EMessenger() = default;
            ~EMessenger() = default;
            EMessenger(const EMessenger&) = delete;
            EMessenger& operator=(const EMessenger&) = delete;

			template<typename MsgT> static void sendMessage(MsgT& msg);
		
	private:
		entt::dispatcher mDispatcher;
	};

	template<typename MsgT> static void EMessenger::sendMessage(MsgT& msg) {
		// TODO - static assert MsgT is child of EMessage
		mDispatcher.enqueue<MsgT>(msg);
	}

}

#endif
