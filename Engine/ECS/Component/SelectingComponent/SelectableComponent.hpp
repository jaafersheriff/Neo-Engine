#pragma once

#include "ECS/Component/Component.hpp"

// #include "ECS/Messaging/Message.hpp"

#include <inttypes.h>

namespace neo {
    static uint32_t sSelectableCounter;

    /* An entity was selected */
    // struct ComponentSelectedMessage : public Message {
    //     const uint32_t mComponentID;
    //     ComponentSelectedMessage(const uint32_t id)
    //         : mComponentID(id)
    //     {}
    // };

    struct SelectableComponent : public Component {
    public:
        SelectableComponent() :
            mID(sSelectableCounter++)
        {}

        uint32_t mID = 0;

        virtual std::string getName() const override {
            return "SelectableComponent";
        }
    };
}