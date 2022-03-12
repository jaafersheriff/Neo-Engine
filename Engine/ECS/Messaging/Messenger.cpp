#include "Messenger.hpp"

#include "Util/Util.hpp"

#include <microprofile.h>

namespace neo {

    entt::dispatcher Messenger::mDispatcher;

    void Messenger::relayMessages(ECS& ecs) {
        MICROPROFILE_SCOPEI("Messenger", "relayMessages()", MP_AUTO);
        NEO_UNUSED(ecs);
        mDispatcher.update();
    }

    void Messenger::clean() {
    //     mMessages.clear();
    //     mReceivers.clear();
    }
}