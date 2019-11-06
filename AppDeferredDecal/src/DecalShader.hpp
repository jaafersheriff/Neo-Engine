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
            decalFBO->generate();
            decalFBO->attachColorTexture(Window::getFrameSize(), 4, TextureFormat{ GL_RGBA, GL_RGBA, GL_NEAREST, GL_REPEAT }); // color
            decalFBO->initDrawBuffers();

            // Handle frame size changing
            Messenger::addReceiver<WindowFrameSizeMessage>(nullptr, [&](const Message &msg) {
                const WindowFrameSizeMessage & m(static_cast<const WindowFrameSizeMessage &>(msg));
                glm::uvec2 frameSize = (static_cast<const WindowFrameSizeMessage &>(msg)).frameSize;
                Library::getFBO("decals")->resize(frameSize);
            });
        }

        virtual void render(const CameraComponent &camera) override {
            auto fbo = Library::getFBO("decals");
            fbo->bind();
            CHECK_GL(glClearColor(0.f, 0.f, 0.f, 1.f));
            CHECK_GL(glClear(GL_COLOR_BUFFER_BIT));

            CHECK_GL(glDisable(GL_CULL_FACE));

            bind();

            loadUniform("P", camera.getProj());
            loadUniform("invPV", glm::inverse(camera.getProj() * camera.getView()));
            loadUniform("V", camera.getView());

            /* Bind gbuffer */
            auto gbuffer = Library::getFBO("gbuffer");
            gbuffer->mTextures[0]->bind();
            loadUniform("gNormal", gbuffer->mTextures[0]->mTextureID);
            gbuffer->mTextures[2]->bind();
            loadUniform("gDepth", gbuffer->mTextures[2]->mTextureID);

            /* Render decals */
            for (auto& decal : Engine::getComponentTuples<DecalRenderable, SpatialComponent, DiffuseMapComponent>()) {
                auto spatial = decal->get<SpatialComponent>();
                loadUniform("M", spatial->getModelMatrix());
                loadUniform("invM", glm::inverse(spatial->getModelMatrix()));

                auto diffuseMap = decal->get<DiffuseMapComponent>();
                auto texture = (const Texture2D *)(diffuseMap->mTexture);
                texture->bind();
                loadUniform("decalTexture", texture->mTextureID);

                Library::getMesh("cube")->draw();
            }

            unbind();
    }
};
