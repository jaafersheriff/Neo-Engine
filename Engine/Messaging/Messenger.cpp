#include "Messenger.hpp"

#include "Util/Util.hpp"
#include "Util/Profiler.hpp"

namespace neo {

    entt::dispatcher Messenger::mDispatcher;

    void Messenger::relayMessages(ECS& ecs) {
        NEO_UNUSED(ecs);
        TRACY_ZONE();
        mDispatcher.update();
    }

    void Messenger::clean() {
        mDispatcher.clear();
        mDispatcher = entt::dispatcher();
    }
}
