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
            // Nothing enqueues into the dispatcher except relayMessages. A send stages the message
            // here first, mirroring the load-queue pattern in ResourceManagerInterface: producers
            // lock only long enough to append, and the consumer swaps the whole queue out and then
            // works on it unlocked.
            //
            // This buys two things. Sending becomes safe from any thread, which is what the render
            // thread needs. And it closes a hazard in entt itself: dispatcher::enqueue push_backs
            // into the very vector dispatcher::publish is walking by reference, so a handler that
            // sends a message of the type being published could reallocate that vector mid-publish.
            // Staging means the dispatcher only ever receives messages from relayMessages, before
            // publishing starts.
            static constexpr size_t kMaxMessageSize = 64;

            struct PendingMessage {
                // Moves the staged message into the dispatcher, then destroys the staged copy.
                void (*mDrain)(entt::dispatcher& dispatcher, void* storage) = nullptr;
                // Destroys the staged copy without delivering it, for clean().
                void (*mDiscard)(void* storage) = nullptr;
                alignas(alignof(std::max_align_t)) std::byte mStorage[kMaxMessageSize] = {};
            };

            static entt::dispatcher mDispatcher;
            static std::mutex mQueueMutex;
            // A deque rather than a vector: growth must not relocate a staged message, which is not
            // trivially copyable (Message has a virtual destructor).
            static std::deque<PendingMessage> mQueue;
    };

    template <typename MsgT, typename... Args>
    void Messenger::sendMessage(Args &&... args) {
        static_assert(sizeof(MsgT) <= kMaxMessageSize, "Message is too large - raise kMaxMessageSize");
        static_assert(alignof(MsgT) <= alignof(std::max_align_t), "Message is over-aligned");

        // Constructed under the lock so it cannot be half-built when relayMessages swaps the queue.
        // Both thunks are captureless lambdas, so they decay to plain function pointers.
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
