#pragma once

#include "Shader/Shader.hpp"
#include "GLObjects/GLHelper.hpp"
#include "Window/Window.hpp"

#include "Messaging/Messenger.hpp"

#include "Engine.hpp"

using namespace neo;

class GBufferSkyboxShader : public Shader {

    public:

        GBufferSkyboxShader(const std::string& vert, const std::string& frag) :
            Shader("GbufferSkybox", vert, frag) 
        { }

        virtual void render(const CameraComponent &camera) override {
            auto fbo = Library::getFBO("gbuffer");
            fbo->bind();

            auto skybox = Engine::getComponentTuple<renderable::SkyboxComponent, CubeMapComponent>();
            if (!skybox) {
                return;
            }

            CHECK_GL(glDisable(GL_CULL_FACE));
            CHECK_GL(glDepthFunc(GL_LEQUAL));
            bind();

            /* Load PV */
            loadUniform("P", camera.getProj());
            loadUniform("V", camera.getView());

            /* Bind texture */
            loadTexture("cubeMap", skybox->get<CubeMapComponent>()->mTexture);

            /* Draw */
            Library::getMesh("cube")->draw();

            unbind();

        }
};