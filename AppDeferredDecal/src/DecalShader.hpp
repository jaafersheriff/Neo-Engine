#pragma once

#include "Shader/Shader.hpp"
#include "GLHelper/GLHelper.hpp"
#include "MasterRenderer/MasterRenderer.hpp"

#include "Loader/Loader.hpp"
#include "NeoEngine.hpp"

using namespace neo;

class DecalShader : public Shader {

    public:

        DecalShader(const std::string &vert, const std::string &frag) :
            Shader("DecalShader", vert, frag) 
        {
            // Create render target
            auto decalFBO = Loader::getFBO("decals");
            decalFBO->generate();
            decalFBO->attachColorTexture(Window::getFrameSize(), 4, GL_RGBA, GL_RGBA, GL_NEAREST, GL_REPEAT); // color
            decalFBO->initDrawBuffers();

            // Handle frame size changing
            Messenger::addReceiver<WindowFrameSizeMessage>(nullptr, [&](const Message &msg) {
                const WindowFrameSizeMessage & m(static_cast<const WindowFrameSizeMessage &>(msg));
                glm::uvec2 frameSize = (static_cast<const WindowFrameSizeMessage &>(msg)).frameSize;
                Loader::getFBO("decals")->resize(frameSize);
            });
        }

        virtual void render(const CameraComponent &camera) override {
            auto fbo = Loader::getFBO("decals");
            fbo->bind();
            CHECK_GL(glClear(GL_COLOR_BUFFER_BIT));

            CHECK_GL(glDisable(GL_CULL_FACE));

            bind();

            loadUniform("P", camera.getProj());
            loadUniform("invPV", glm::inverse(camera.getProj() * camera.getView()));
            loadUniform("V", camera.getView());

            /* Bind gbuffer */
            auto gbuffer = Loader::getFBO("gbuffer");
            gbuffer->mTextures[0]->bind();
            loadUniform("gNormal", gbuffer->mTextures[0]->mTextureID);
            gbuffer->mTextures[2]->bind();
            loadUniform("gDepth", gbuffer->mTextures[2]->mTextureID);

            /* Render decals */
            for (auto& decal : MasterRenderer::getRenderables<DecalShader, RenderableComponent>()) {
                auto& mesh = decal->getMesh();
                CHECK_GL(glBindVertexArray(mesh.mVAOID));
                CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.mElementBufferID));
 
                auto spat = decal->getGameObject().getSpatial();
                loadUniform("M", spat->getModelMatrix());
                loadUniform("invM", glm::inverse(spat->getModelMatrix()));

                auto diffuseMap = decal->getGameObject().getComponentByType<DiffuseMapComponent>();
                if (diffuseMap) {
                    auto texture = (const Texture2D *)(diffuseMap->mTexture);
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
