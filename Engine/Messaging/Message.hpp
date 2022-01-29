/* Message struct used to communicate data between components */
#pragma once

#include "glm/glm.hpp"

#include <memory>
#include <typeindex>
// Don't add includes. If possible, forward declard. This file shouldn't contain
// any functionality, and it will be included all over the place.

//==============================================================================
// README !!!
//------------------------------------------------------------------------------ 
// A message can be sent from anywhere using Scene's sendMessage method, which
// looks like...
//
//     Scene::sendMessage<MessageType>([nullptr | gameobject], message args);
//
// This message is sent out to all receivers of the specified message type. If
// gameobject is not null, the message is ALSO sent to any receiver of that
// game object and message type. This is for efficient inter-component
// communication.
//
// The scene keeps a list of messages, and what type they were, until it's time
// to relay the messages. This happens before and after every system update.
// The system will only relay messages to any receivers of the corresponding
// type. A receiver is a std::function<void (const Message &)> . To add a
// receiver, do...
//
//     Scene::addReceiver<MessageType>([nullptr | gameobject], receiver);
//
// Here, if gameobject is null, the receiver will receive all messages of the
// specified type. If gameobject is not null, the receiver will only receive
// messages of the specified type that have been sent to that object. This is
// how you do efficient inter-component communication.
//
// For receiver, can pass either a function pointer or a lambda. Note, you can
// technically bind a method to a std::function, but it's ugly. For adding
// receivers from objects that can reference that object, I reccommend using a
// lambda, like so...
//    
//    auto receiver = [&](const Message & msg_) {
//        const MessageIWantType & msg(static_cast<const MessageIWantType &>(msg_));
//        ...
//    };
//    Scene::addReceiver<MessageIWantType>(receiver);
//
//------------------------------------------------------------------------------


namespace neo {

    class GameObject;
    class Component;
    class SpatialComponent;
    class CameraComponent;


    struct Message {
        virtual ~Message() = default;
    };




    /* A spatiality was changed in some way */
    struct SpatialChangeMessage : public Message {
        const SpatialComponent & spatial;
        SpatialChangeMessage(const SpatialComponent & spatial) : spatial(spatial) {}
    };

    /* The window was resized */
    struct WindowFrameSizeMessage : public Message {
        glm::uvec2 frameSize;
        WindowFrameSizeMessage(const glm::uvec2 & frameSize) : frameSize(frameSize) {}
    };



    // // key input
    // struct KeyMessage : public Message {
    //     int key, action, mods;
    //     KeyMessage(int key, int action, int mods) : key(key), action(action), mods(mods) {}
    // };

    // // mouse input
    // struct MouseMessage : public Message {
    //     int button, action, mods;
    //     MouseMessage(int button, int action, int mods) : button(button), action(action), mods(mods) {}
    // };

    // // scroll input
    // struct ScrollMessage : public Message {
    //     float dx, dy;
    //     ScrollMessage(float dx, float dy) : dx(dx), dy(dy) {}
    // };



    // // component has put through the init queue and added to the scene
    // struct ComponentAddedMessage : public Message {
    //     Component & comp;
    //     std::type_index typeI;
    //     ComponentAddedMessage(Component & comp, std::type_index typeI) : comp(comp), typeI(typeI) {}
    // };

    // // component has been added to kill queue and will be removed from the scene
    // struct ComponentRemovedMessage : public Message {
    //     std::unique_ptr<Component> comp;
    //     std::type_index typeI;
    //     ComponentRemovedMessage(std::unique_ptr<Component> && comp, std::type_index typeI) : comp(std::move(comp)), typeI(typeI) {}
    // };


}
