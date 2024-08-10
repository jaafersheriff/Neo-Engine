#include "Messenger.hpp"

#include "Util/Util.hpp"
#include "Util/Profiler.hpp"

#include <tracy/Tracy.hpp>

namespace neo {

    entt::dispatcher Messenger::mDispatcher;
    std::mutex Messenger::mDisaptcherMutex;

    void Messenger::relayMessages(ECS& ecs) {
        NEO_UNUSED(ecs);
        TRACY_ZONE();
        std::lock_guard<std::mutex> lock(mDisaptcherMutex);
        mDispatcher.update();
    }

    void Messenger::clean() {
        mDispatcher.clear();
        mDispatcher = entt::dispatcher();
    }
}