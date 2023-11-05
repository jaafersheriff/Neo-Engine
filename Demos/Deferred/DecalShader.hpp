#pragma once

#include "Loader/Library.hpp"
#include "Renderer/Shader/Shader.hpp"
#include "Renderer/GLObjects/GLHelper.hpp"
#include "Renderer/GLObjects/Framebuffer.hpp"

#include "DecalRenderable.hpp"

#include "ECS/ECS.hpp"
#include "Messaging/Messenger.hpp"
#include "ECS/Component/CameraComponent/MainCameraComponent.hpp"
#include "ECS/Component/CameraComponent/CameraComponent.hpp"
#include "ECS/Component/SpatialComponent/SpatialComponent.hpp"

using namespace neo;

namespace Deferred {
    class DecalShader : public Shader {

    public:

        DecalShader(const std::string& vert, const std::string& frag) :
            Shader("DecalShader", vert, frag) {
            // Create render target
            auto decalFBO = Library::createFBO("decals");
            decalFBO->attachColorTexture({ 1, 1 }, TextureFormat{ GL_RGBA, GL_RGBA, GL_NEAREST, GL_REPEAT }); // color
            decalFBO->initDrawBuffers();

            // Handle frame size changing
            Messenger::addReceiver<FrameSizeMessage, &DecalShader::_onFrameSizeChanged>(this);
        }

        virtual void render(const ECS& ecs) override {
            auto fbo = Library::getFBO("decals");
            fbo->bind();
            glClearColor(0.f, 0.f, 0.f, 1.f);
            glClear(GL_COLOR_BUFFER_BIT);

            glDisable(GL_CULL_FACE);

            bind();
            if (auto camera = ecs.getSingleView<MainCameraComponent, SpatialComponent>()) {
                auto&& [cameraEntity, _, cameraSpatial] = *camera;
                loadUniform("P", ecs.cGetComponentAs<CameraComponent, PerspectiveCameraComponent>(cameraEntity)->getProj());
                loadUniform("invP", glm::inverse(ecs.cGetComponentAs<CameraComponent, PerspectiveCameraComponent>(cameraEntity)->getProj()));
                loadUniform("V", cameraSpatial.getView());
            }

            /* Bind gbuffer */
            auto gbuffer = Library::getFBO("gbuffer");
            loadTexture("gNormal", *gbuffer->mTextures[0]);
            loadTexture("gDepth", *gbuffer->mTextures[2]);

            /* Render decals */
            ecs.getView<DecalRenderable, SpatialComponent>().each([this](ECS::Entity entity, const DecalRenderable& decal, const SpatialComponent& spatial) {
                NEO_UNUSED(entity);
                loadUniform("M", spatial.getModelMatrix());
                loadUniform("invM", glm::inverse(spatial.getModelMatrix()));

                loadTexture("decalTexture", *decal.mDiffuseMap);
                Library::getMesh("cube").mMesh->draw();

            });

            unbind();
        }

    private:
        void _onFrameSizeChanged(const FrameSizeMessage& msg) {
                Library::getFBO("decals")->resize(msg.mSize);
        }
    };
}
