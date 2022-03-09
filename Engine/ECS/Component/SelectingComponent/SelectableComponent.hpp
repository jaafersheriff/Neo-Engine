#pragma once

#include "ECS/Component/Component.hpp"

#include "ECS/Messaging/Message.hpp"

#include <inttypes.h>

namespace neo {

    /* An entity was selected */
    struct ComponentSelectedMessage : public Message {
        const uint32_t mComponentID;
        ComponentSelectedMessage(const uint32_t id)
            : mComponentID(id)
        {}
    };

    class SelectableComponent : public Component {
    public:

        SelectableComponent(GameObject* go) :
            Component(go),
            mID(sCounter++)
        {}

        uint32_t mID = 0;

    private:
        static uint32_t sCounter;
    };
}