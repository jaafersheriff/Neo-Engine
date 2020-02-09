#pragma once

#include "Shader/Shader.hpp"
#include "GLObjects/GLHelper.hpp"

#include "Engine.hpp"

#include "DecalRenderable.hpp"

using namespace neo;

class DecalShader : public Shader {

    public:

        DecalShader(const std::string &vert, const std::string &frag) :
            Shader("DecalShader", vert, frag) {
            // Create render target
            auto decalFBO = Library::getFBO("decals");
            decalFBO->attachColorTexture(Window::getFrameSize(), TextureFormat{ GL_RGBA, GL_RGBA, GL_NEAREST, GL_REPEAT }); // color
            decalFBO->initDrawBuffers();

            // Handle frame size changing
            Messenger::addReceiver<WindowFrameSizeMessage>(nullptr, [&](const Message &msg) {
                const WindowFrameSizeMessage & m(static_cast<const WindowFrameSizeMessage &>(msg));
                glm::uvec2 frameSize = (static_cast<const WindowFrameSizeMessage &>(msg)).frameSize;
                Library::getFBO("decals")->resize(frameSize);
            });
        }

        virtual void render() override {
            auto fbo = Library::getFBO("decals");
            fbo->bind();
            CHECK_GL(glClearColor(0.f, 0.f, 0.f, 1.f));
            CHECK_GL(glClear(GL_COLOR_BUFFER_BIT));

            CHECK_GL(glDisable(GL_CULL_FACE));

            bind();
            if (auto camera = Engine::getComponentTuple<MainCameraComponent, CameraComponent>()) {
                loadUniform("P", camera->get<CameraComponent>()->getProj());
                loadUniform("invPV", glm::inverse(camera->get<CameraComponent>()->getProj() * camera->get<CameraComponent>()->getView()));
                loadUniform("V", camera->get<CameraComponent>()->getView());
            }

            /* Bind gbuffer */
            auto gbuffer = Library::getFBO("gbuffer");
            loadTexture("gNormal", *gbuffer->mTextures[0]);
            loadTexture("gDepth",  *gbuffer->mTextures[2]);

            /* Render decals */
            for (auto& decal : Engine::getComponentTuples<DecalRenderable, SpatialComponent, DiffuseMapComponent>()) {
                auto spatial = decal->get<SpatialComponent>();
                loadUniform("M", spatial->getModelMatrix());
                loadUniform("invM", glm::inverse(spatial->getModelMatrix()));

                auto diffuseMap = decal->get<DiffuseMapComponent>();
                loadTexture("decalTexture", diffuseMap->mTexture);

                Library::getMesh("cube")->draw();
            }

            unbind();
    }
};
