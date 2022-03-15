#pragma once

#include "ECS/Component/Component.hpp"

#include "ECS/Messaging/Message.hpp"

#include <inttypes.h>

namespace neo {

    /* An entity was selected */
    struct EntitySelectedMessage : public Message {
        ECS::Entity mEntity;
        EntitySelectedMessage(const uint32_t id)
            : mEntity(static_cast<ECS::Entity>(id))
        {}
    };

    struct SelectableComponent : public Component {
    public:
        SelectableComponent()
        {}

        virtual std::string getName() const override {
            return "SelectableComponent";
        }
    };
}