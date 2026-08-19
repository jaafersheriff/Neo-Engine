/* Message struct used to communicate data between components */
#pragma once

#include "glm/glm.hpp"

#include "ECS/ECS.hpp"
#include "ResourceManager/ResourceManagerInterface.hpp"

#include <memory>
#include <typeindex>
#include <functional>
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

    class Texture;
    using TextureHandle = ResourceHandle<Texture>;

    // TODO - just make this a system with single frame components
    struct Component;
    struct SpatialComponent;

    struct Message {
        virtual ~Message() = default;
    };

    /* A spatiality was changed in some way */
    struct SpatialChangeMessage : public Message {
        const SpatialComponent & mSpatial;
        SpatialChangeMessage(const SpatialComponent & spatial) : mSpatial(spatial) {}
    };

    /* The window was resized */
    struct FrameSizeMessage : public Message {
        glm::uvec2 mSize;
        FrameSizeMessage(const glm::uvec2 & frameSize) : mSize(frameSize) {}
    };

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



    //==========================================================================
    // World-mutating messages
    //--------------------------------------------------------------------------
    // These say "the renderer discovered something the world needs to remember". The renderer only
    // ever holds a *clone* of the ECS, so writing the discovery down directly would be thrown away
    // when the clone is next refilled - it has to travel back as a message.
    //
    // Sent and received exactly like every other message: Messenger::sendMessage to send, and the
    // ECS registers itself as the receiver in ECS::_init, the same remove-then-add that Keyboard,
    // Mouse and WindowSurface do in theirs. The handlers live in ECS.cpp.

    /* The renderer finished convolving an IBL's skybox cubemap */
    struct IBLConvolvedMessage : public Message {
        ECS::Entity mEntity;
        TextureHandle mConvolvedSkybox;
        IBLConvolvedMessage(ECS::Entity entity, TextureHandle convolvedSkybox)
            : mEntity(entity)
            , mConvolvedSkybox(convolvedSkybox)
        {}
    };

    /* The renderer finished generating an IBL's DFG LUT */
    struct IBLDFGLutGeneratedMessage : public Message {
        ECS::Entity mEntity;
        TextureHandle mDFGLut;
        IBLDFGLutGeneratedMessage(ECS::Entity entity, TextureHandle dfgLut)
            : mEntity(entity)
            , mDFGLut(dfgLut)
        {}
    };

    /* A line mesh's nodes were uploaded to its mesh, so it is no longer dirty */
    struct LineMeshUploadedMessage : public Message {
        ECS::Entity mEntity;
        LineMeshUploadedMessage(ECS::Entity entity)
            : mEntity(entity)
        {}
    };
}
