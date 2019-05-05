#pragma once

#include "Shader/Shader.hpp"
#include "GLObjects/GLHelper.hpp"
#include "Renderer/Renderer.hpp"

#include "Engine.hpp"

#include "DecalRenderable.hpp"

class DecalShader : public neo::Shader {

    public:

        DecalShader(const std::string &vert, const std::string &frag) :
            neo::Shader("DecalShader", vert, frag) {
            // Create render target
            auto decalFBO = neo::Library::getFBO("decals");
            decalFBO->generate();
            decalFBO->attachColorTexture(neo::Window::getFrameSize(), 4, neo::TextureFormat{ GL_RGBA, GL_RGBA, GL_NEAREST, GL_REPEAT }); // color
            decalFBO->initDrawBuffers();

            // Handle frame size changing
            neo::Messenger::addReceiver<neo::WindowFrameSizeMessage>(nullptr, [&](const neo::Message &msg) {
                const neo::WindowFrameSizeMessage & m(static_cast<const neo::WindowFrameSizeMessage &>(msg));
                glm::uvec2 frameSize = (static_cast<const neo::WindowFrameSizeMessage &>(msg)).frameSize;
                neo::Library::getFBO("decals")->resize(frameSize);
            });
        }

        virtual void render(const neo::CameraComponent &camera) override {
            auto fbo = neo::Library::getFBO("decals");
            fbo->bind();
            CHECK_GL(glClearColor(0.f, 0.f, 0.f, 1.f));
            CHECK_GL(glClear(GL_COLOR_BUFFER_BIT));

            CHECK_GL(glDisable(GL_CULL_FACE));

            bind();

            loadUniform("P", camera.getProj());
            loadUniform("invPV", glm::inverse(camera.getProj() * camera.getView()));
            loadUniform("V", camera.getView());

            /* Bind gbuffer */
            auto gbuffer = neo::Library::getFBO("gbuffer");
            gbuffer->mTextures[0]->bind();
            loadUniform("gNormal", gbuffer->mTextures[0]->mTextureID);
            gbuffer->mTextures[2]->bind();
            loadUniform("gDepth", gbuffer->mTextures[2]->mTextureID);

            /* Render decals */
            for (auto& decal : neo::Engine::getComponents<DecalRenderable>()) {
                auto& mesh = *neo::Library::getMesh("cube");
                CHECK_GL(glBindVertexArray(mesh.mVAOID));
                CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.mElementBufferID));
 
                auto spat = decal->getGameObject().getSpatial();
                loadUniform("M", spat->getModelMatrix());
                loadUniform("invM", glm::inverse(spat->getModelMatrix()));

                auto diffuseMap = decal->getGameObject().getComponentByType<neo::DiffuseMapComponent>();
                if (diffuseMap) {
                    auto texture = (const neo::Texture2D *)(diffuseMap->mTexture);
                    texture->bind();
                    loadUniform("decalTexture", texture->mTextureID);
                }

                mesh.draw();
            }

            CHECK_GL(glBindVertexArray(0));
            CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, 0));
            CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
            unbind();
            CHECK_GL(glCullFace(GL_BACK));
    }
};
