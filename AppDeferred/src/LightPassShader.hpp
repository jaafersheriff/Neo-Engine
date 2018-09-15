#pragma once

#include "Shader/Shader.hpp"
#include "GLHelper/GLHelper.hpp"
#include "MasterRenderer/MasterRenderer.hpp"

#include "Loader/Loader.hpp"
#include "NeoEngine.hpp"

using namespace neo;

class LightPassShader : public Shader {

    public:

        bool showLights = false;
        float showRadius = 0.1f;

        LightPassShader(const std::string &vert, const std::string &frag) :
            Shader("LightPassShader", vert, frag) 
        {
            // Create render target
            auto lightFBO = Loader::getFBO("lightpass");
            lightFBO->generate();
            lightFBO->attachColorTexture(Window::getFrameSize(), 4, GL_RGBA, GL_RGBA, GL_NEAREST, GL_REPEAT); // color
            lightFBO->attachDepthTexture(Window::getFrameSize(), GL_NEAREST, GL_REPEAT); // depth

            // Handle frame size changing
            Messenger::addReceiver<WindowFrameSizeMessage>(nullptr, [&](const Message &msg) {
                const WindowFrameSizeMessage & m(static_cast<const WindowFrameSizeMessage &>(msg));
                glm::ivec2 frameSize = (static_cast<const WindowFrameSizeMessage &>(msg)).frameSize;
                auto lightFBO = Loader::getFBO("lightpass");
                lightFBO->textures[0]->width  = lightFBO->textures[1]->width  = frameSize.x;
                lightFBO->textures[0]->height = lightFBO->textures[1]->height = frameSize.y;
                lightFBO->textures[0]->bind();
                CHECK_GL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, frameSize.x, frameSize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr));
                lightFBO->textures[1]->bind();
                CHECK_GL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, frameSize.x, frameSize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr));
            });
        }

        virtual void render(const CameraComponent &camera) override {
            auto fbo = Loader::getFBO("lightpass");
            fbo->bind();
            CHECK_GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

            bind();

            CHECK_GL(glBlendFunc(GL_ONE, GL_ONE));
            CHECK_GL(glDisable(GL_DEPTH_TEST));
            CHECK_GL(glEnable(GL_CULL_FACE));

            loadUniform("showLights", showLights);
            loadUniform("showRadius", showRadius);

            loadUniform("P", camera.getProj());
            loadUniform("V", camera.getView());
            loadUniform("invP", glm::inverse(camera.getProj()));
            loadUniform("invV", glm::inverse(camera.getView()));
            loadUniform("camPos", camera.getGameObject().getSpatial()->getPosition());

            /* Bind sphere volume */
            auto mesh = Loader::getMesh("ico_2", true);
            CHECK_GL(glBindVertexArray(mesh->vaoId));
            CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->eleBufId));

            /* Bind gbuffer */
            auto gbuffer = Loader::getFBO("gbuffer");
            gbuffer->textures[0]->bind();
            loadUniform("gNormal", gbuffer->textures[0]->textureId);
            gbuffer->textures[1]->bind();
            loadUniform("gDiffuse", gbuffer->textures[1]->textureId);
            gbuffer->textures[2]->bind();
            loadUniform("gDepth", gbuffer->textures[2]->textureId);

            /* Render light volumes */
            for (auto & light : NeoEngine::getComponents<LightComponent>()) {
                auto spat = light->getGameObject().getSpatial();
                loadUniform("M", spat->getModelMatrix());
                loadUniform("lightPos", spat->getPosition());
                loadUniform("lightRadius", spat->getScale().x);
                loadUniform("lightCol", light->getColor());

                // If camera is inside light 
                float dist = glm::distance(spat->getPosition(), camera.getGameObject().getSpatial()->getPosition());
                if (dist - camera.getNear() < spat->getScale().x) {
                    CHECK_GL(glCullFace(GL_FRONT));
                }
                else {
                    CHECK_GL(glCullFace(GL_BACK));
                }
                mesh->draw();
            }

            CHECK_GL(glBindVertexArray(0));
            CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, 0));
            CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
            unbind();
            CHECK_GL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
            CHECK_GL(glEnable(GL_DEPTH_TEST));
            CHECK_GL(glEnable(GL_CULL_FACE));
            CHECK_GL(glCullFace(GL_BACK));
    }
};
