#pragma once

#include "Message.hpp"

#include <vector>
#include <unordered_map>
#include <typeindex>
#include <memory>
#include <deque>
#include <functional>
#include <mutex>
#include <new>
#include <type_traits>
#include <ext/entt_incl.hpp>
#include <entt/signal/dispatcher.hpp>

namespace neo {

	 class ECS;

	 class Messenger {
		  public:
				Messenger() = default;
				~Messenger() = default;
				Messenger(const Messenger&) = delete;
				Messenger& operator=(const Messenger&) = delete;

				template <typename MsgT, typename... Args> static void sendMessage(Args &&... args);
				template<typename MsgT, auto Func, typename Caller> static void addReceiver(Caller &&caller);
				template<typename MsgT, auto Func, typename Caller> static void removeReceiver(Caller &&caller);
				template<typename MsgT, typename Caller> static void removeReceiver(Caller &&caller);

				static void relayMessages(ECS& ecs);
				static void clean();

		  private:
				static constexpr size_t kMaxMessageSize = 64;

				struct PendingMessage {
					// Moves the staged message into the dispatcher, then destroys the staged copy.
					void (*mDrain)(entt::dispatcher& dispatcher, void* storage) = nullptr;
					// Destroys the staged copy without delivering it, for clean().
					void (*mDiscard)(void* storage) = nullptr;

					// Actual Message is stored here and extracted via template typed casting
					alignas(alignof(std::max_align_t)) std::byte mStorage[kMaxMessageSize] = {};
				};

				static entt::dispatcher mDispatcher;
				static std::mutex mQueueMutex;
				static std::deque<PendingMessage> mQueue;
	 };

	 template <typename MsgT, typename... Args>
	 void Messenger::sendMessage(Args &&... args) {
		  static_assert(sizeof(MsgT) <= kMaxMessageSize, "Message is too large - raise kMaxMessageSize");
		  static_assert(alignof(MsgT) <= alignof(std::max_align_t), "Message is over-aligned");

		  std::lock_guard<std::mutex> lock(mQueueMutex);
		  PendingMessage& pending = mQueue.emplace_back();
		  pending.mDrain = [](entt::dispatcher& dispatcher, void* storage) {
				MsgT* message = static_cast<MsgT*>(storage);
				dispatcher.enqueue<MsgT>(std::move(*message));
				message->~MsgT();
		  };
		  pending.mDiscard = [](void* storage) {
				static_cast<MsgT*>(storage)->~MsgT();
		  };
		  ::new (static_cast<void*>(pending.mStorage)) MsgT{ std::forward<Args>(args)... };
	 }

	 template<typename MsgT, auto Func, typename Caller> 
	 void Messenger::addReceiver(Caller &&caller) {
		  mDispatcher.sink<MsgT>().connect<Func>(caller);
	 }
	 
	 template<typename MsgT, auto Func, typename Caller> 
	 void Messenger::removeReceiver(Caller&& caller) {
		  mDispatcher.sink<MsgT>().disconnect<Func>(caller);
	 }

	 template<typename MsgT, typename Caller> 
	 void Messenger::removeReceiver(Caller&& caller) {
		  mDispatcher.sink<MsgT>().disconnect(caller);
	 }
}
