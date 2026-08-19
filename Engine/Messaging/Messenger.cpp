#include "Messenger.hpp"

#include "Util/Util.hpp"
#include "Util/Profiler.hpp"

namespace neo {

	 // Deliberately unlocked in case a Handler sends another message, which will then land on the pending queue
	 entt::dispatcher Messenger::mDispatcher;

	 std::mutex Messenger::mQueueMutex;
	 std::deque<Messenger::PendingMessage> Messenger::mQueue;

	 void Messenger::relayMessages(ECS& ecs) {
		  NEO_UNUSED(ecs);
		  TRACY_ZONE();

		  std::deque<PendingMessage> pending;
		  {
				std::lock_guard<std::mutex> lock(mQueueMutex);
				std::swap(mQueue, pending);
		  }
		  for (PendingMessage& message : pending) {
				message.mDrain(mDispatcher, message.mStorage);
		  }

		  mDispatcher.update();
	 }

	 void Messenger::clean() {
		  {
				std::lock_guard<std::mutex> lock(mQueueMutex);
				for (PendingMessage& message : mQueue) {
					 message.mDiscard(message.mStorage);
				}
				mQueue.clear();
		  }
		  mDispatcher.clear();
		  mDispatcher = entt::dispatcher();
	 }
}
