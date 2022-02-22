#pragma once

#include "Loader/Library.hpp"
#include "Renderer/Shader/Shader.hpp"
#include "Renderer/GLObjects/GLHelper.hpp"
#include "Renderer/GLObjects/Framebuffer.hpp"

#include "DecalRenderable.hpp"

#include "ECS/ECS.hpp"
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
            Messenger::addReceiver<WindowFrameSizeMessage>(nullptr, [&](const Message& msg, ECS& ecs) {
                NEO_UNUSED(ecs);
                glm::uvec2 frameSize = (static_cast<const WindowFrameSizeMessage&>(msg)).mFrameSize;
                Library::getFBO("decals")->resize(frameSize);
                });
        }

        virtual void render(const ECS& ecs) override {
            auto fbo = Library::getFBO("decals");
            fbo->bind();
            CHECK_GL(glClearColor(0.f, 0.f, 0.f, 1.f));
            CHECK_GL(glClear(GL_COLOR_BUFFER_BIT));

            CHECK_GL(glDisable(GL_CULL_FACE));

            bind();
            if (auto camera = ecs.getComponentTuple<MainCameraComponent, CameraComponent>()) {
                loadUniform("P", camera->get<CameraComponent>()->getProj());
                loadUniform("invPV", glm::inverse(camera->get<CameraComponent>()->getProj() * camera->get<CameraComponent>()->getView()));
                loadUniform("V", camera->get<CameraComponent>()->getView());
            }

            /* Bind gbuffer */
            auto gbuffer = Library::getFBO("gbuffer");
            loadTexture("gNormal", *gbuffer->mTextures[0]);
            loadTexture("gDepth", *gbuffer->mTextures[2]);

            /* Render decals */
            for (auto& decal : ecs.getComponentTuples<DecalRenderable, SpatialComponent>()) {
                auto spatial = decal->get<SpatialComponent>();
                loadUniform("M", spatial->getModelMatrix());
                loadUniform("invM", glm::inverse(spatial->getModelMatrix()));

                loadTexture("decalTexture", decal->get<DecalRenderable>()->mDiffuseMap);

                Library::getMesh("cube").mMesh->draw();
            }

            unbind();
        }
    };
}
