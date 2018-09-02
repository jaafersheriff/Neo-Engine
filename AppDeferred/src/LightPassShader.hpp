#pragma once

#include "Shader/Shader.hpp"
#include "GLHelper/GLHelper.hpp"
#include "MasterRenderer/MasterRenderer.hpp"

#include "Loader/Loader.hpp"
#include "NeoEngine.hpp"

using namespace neo;

class LightPassShader : public Shader {

    public:

        // TODO : add this to post process stack
        LightPassShader(const std::string &vert, const std::string &frag) :
            Shader("LightPassShader", vert, frag) 
        {
        }

        virtual void render(const CameraComponent &camera) override {
            bind();

            CHECK_GL(glBlendFunc(GL_ONE, GL_ONE));
            CHECK_GL(glDisable(GL_DEPTH_TEST));
            CHECK_GL(glEnable(GL_CULL_FACE));
            CHECK_GL(glCullFace(GL_FRONT));

            loadUniform("P", camera.getProj());
            loadUniform("V", camera.getView());

            /* Bind sphere volume */
            auto mesh = Loader::getMesh("sphere");
            CHECK_GL(glBindVertexArray(mesh->vaoId));
            CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->eleBufId));

            /* Bind gbuffer */
            auto fbo = MasterRenderer::getFBO("gbuffer");
            fbo->textures[0]->bind();
            loadUniform("gPosition", fbo->textures[0]->textureId);
            fbo->textures[1]->bind();
            loadUniform("gNormal", fbo->textures[1]->textureId);
            fbo->textures[2]->bind();
            loadUniform("gDiffuse", fbo->textures[2]->textureId);
            fbo->textures[3]->bind();
            loadUniform("gSpecular", fbo->textures[3]->textureId);
            fbo->textures[4]->bind();
            loadUniform("gDepth", fbo->textures[4]->textureId);

            /* Render light volumes */
            for (auto light : NeoEngine::getComponents<LightComponent>()) {
                loadUniform("M", light->getGameObject().getSpatial()->getModelMatrix());
                loadUniform("lightPos", light->getGameObject().getSpatial()->getPosition());
                loadUniform("lightCol", light->getColor());
                mesh->draw();
            }

            CHECK_GL(glBindVertexArray(0));
            CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, 0));
            CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
            unbind();
            MasterRenderer::resetState();
    }
};
