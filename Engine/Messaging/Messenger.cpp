#include "Messenger.hpp"

#include "Util/Util.hpp"

#include <tracy/Tracy.hpp>

namespace neo {

    entt::dispatcher Messenger::mDispatcher;

    void Messenger::relayMessages(ECS& ecs) {
        ZoneScoped;
        NEO_UNUSED(ecs);
        mDispatcher.update();
    }

    void Messenger::clean() {
        mDispatcher.clear();
        mDispatcher = entt::dispatcher();
    }
}