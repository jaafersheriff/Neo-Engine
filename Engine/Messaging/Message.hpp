/* Message struct used to communicate data between components */
#pragma once

#include "glm/glm.hpp"

#include "ECS/ECS.hpp"
#include "ResourceManager/ResourceManagerInterface.hpp"

#include <memory>
#include <typeindex>
#include <functional>
// Don't add includes. If possible, forward declare. This file shouldn't contain
// any functionality, and it will be included all over the place.

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
