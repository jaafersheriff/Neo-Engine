#include "Messenger.hpp"

#include "Util/Util.hpp"
#include "Util/Profiler.hpp"

namespace neo {

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

        // Deliberately unlocked. Handlers can do anything, including sending messages - those land
        // in the staging queue and are delivered by the next relay rather than deadlocking or
        // reallocating the dispatcher's storage underneath this call.
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
