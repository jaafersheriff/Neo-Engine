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

        LightPassShader(const std::string &vert, const std::string &frag) :
            Shader("LightPassShader", vert, frag) 
        {}

        virtual void render(const CameraComponent &camera) override {
            bind();

            CHECK_GL(glBlendFunc(GL_ONE, GL_ONE));
            CHECK_GL(glDisable(GL_DEPTH_TEST));
            CHECK_GL(glEnable(GL_CULL_FACE));

            loadUniform("showLights", showLights);

            loadUniform("P", camera.getProj());
            loadUniform("V", camera.getView());
            loadUniform("camPos", camera.getGameObject().getSpatial()->getPosition());

            /* Bind sphere volume */
            auto mesh = Loader::getMesh("sphere.obj", true);
            CHECK_GL(glBindVertexArray(mesh->vaoId));
            CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->eleBufId));

            /* Bind gbuffer */
            auto fbo = MasterRenderer::getFBO("gbuffer");
            fbo->textures[0]->bind();
            loadUniform("gNormal", fbo->textures[0]->textureId);
            fbo->textures[1]->bind();
            loadUniform("gDiffuse", fbo->textures[1]->textureId);
            fbo->textures[2]->bind();
            loadUniform("gDepth", fbo->textures[2]->textureId);

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
